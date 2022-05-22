// authors:
//     Bruno Soares    fc57100
//     Renato Custódio fc56320
//     João Vedor      fc56311



#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/main.h"
#include "../include/configuration.h"
#include "../include/log.h"
#include "../include/main-private.h"
#include "../include/mesignal.h"
#include "../include/metime.h"
#include "../include/process.h"
#include "../include/stats.h"
#include "../include/synchronization.h"

int alarm_id;
struct args *args;
struct main_data *global_data;
struct communication_buffers *global_buffers;
struct semaphores *global_sems;

// Função que verifica se uma string é composta apenas por digitos
int isNumber(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

/* Função que lê os argumentos da aplicação, nomeadamente o número
 * máximo de operações, o tamanho dos buffers de memória partilhada
 * usados para comunicação, e o número de clientes, de motoristas e de
 * restaurantes. Guarda esta informação nos campos apropriados da
 * estrutura main_data.
 */
void main_args(int argc, char *argv[], struct main_data *data) {
    if (argc != 2) {
        printf(
            "Uso: config.txt max_ops buffers_size n_restaurants n_drivers n_clients log_filename statistics_filename "
            "alarme_time\n");
        printf("Exemplo: ./bin/magnaeats config.txt\n");
        destroy_dynamic_memory(data);
        exit(1);
    }

    args = create_dynamic_memory(sizeof(struct args));
    args->log_filename = create_dynamic_memory(100);
    args->statistics_filename = create_dynamic_memory(100);

    if (readFile(argv[1], args) == -1) {
        perror("file_read_error");
        destroy_dynamic_memory(data);
        exit(1);
    }

    data->max_ops = args->max_ops;
    data->buffers_size = args->buffers_size;
    data->n_clients = args->n_clients;
    data->n_restaurants = args->n_restaurants;
    data->n_drivers = args->n_drivers;
}

/* Função que reserva a memória dinâmica necessária para a execução
 * do MAGNAEATS, nomeadamente para os arrays *_pids e *_stats da estrutura
 * main_data. Para tal, pode ser usada a função create_dynamic_memory.
 */
void create_dynamic_memory_buffers(struct main_data *data) {
    data->client_stats = create_dynamic_memory(data->n_clients * sizeof(int));
    data->driver_stats = create_dynamic_memory(data->n_drivers * sizeof(int));
    data->restaurant_stats = create_dynamic_memory(data->n_restaurants * sizeof(int));
    data->client_pids = create_dynamic_memory(data->n_clients * sizeof(int));
    data->driver_pids = create_dynamic_memory(data->n_drivers * sizeof(int));
    data->restaurant_pids = create_dynamic_memory(data->n_restaurants * sizeof(int));
}

/* Função que reserva a memória partilhada necessária para a execução do
 * MAGNAEATS. É necessário reservar memória partilhada para todos os buffers da
 * MAGNAEATS. É necessário reservar memória partilhada para todos os buffers da
 * estrutura communication_buffers, incluindo os buffers em si e respetivos
 * pointers, assim como para o array data->results e variável data->terminate.
 * Para tal, pode ser usada a função create_shared_memory.
 */
void create_shared_memory_buffers(struct main_data *data, struct communication_buffers *buffers) {
    buffers->main_rest->buffer =
        create_shared_memory(STR_SHM_MAIN_REST_BUFFER, data->buffers_size * sizeof(struct operation));
    buffers->main_rest->ptrs = create_shared_memory(STR_SHM_MAIN_REST_PTR, data->buffers_size * sizeof(int));

    buffers->rest_driv->buffer =
        create_shared_memory(STR_SHM_REST_DRIVER_BUFFER, data->buffers_size * sizeof(struct operation));
    buffers->rest_driv->ptrs =
        create_shared_memory(STR_SHM_REST_DRIVER_PTR, data->buffers_size * sizeof(struct pointers));

    buffers->driv_cli->buffer =
        create_shared_memory(STR_SHM_DRIVER_CLIENT_BUFFER, data->buffers_size * sizeof(struct operation));
    buffers->driv_cli->ptrs = create_shared_memory(STR_SHM_DRIVER_CLIENT_PTR, data->buffers_size * sizeof(int));

    data->results = create_shared_memory(STR_SHM_RESULTS, data->max_ops * sizeof(struct operation));
    data->terminate = create_shared_memory(STR_SHM_TERMINATE, sizeof(int));
}

/* Função que inicia os processos dos restaurantes, motoristas e
 * clientes. Para tal, pode usar as funções launch_*,
 * guardando os pids resultantes nos arrays respetivos
 * da estrutura data.
 */
void launch_processes(struct communication_buffers *buffers, struct main_data *data, struct semaphores *sems) {
    stop_signal();
    int i;
    for (i = 0; i < data->n_restaurants; i++) {
        data->restaurant_pids[i] = launch_restaurant(i, buffers, data, sems);
    }

    for (i = 0; i < data->n_drivers; i++) {
        data->driver_pids[i] = launch_driver(i, buffers, data, sems);
    }

    for (i = 0; i < data->n_clients; i++) {
        data->client_pids[i] = launch_client(i, buffers, data, sems);
    }

    alarm_id = launch_alarme(data, args->alarm_time);
}

/* Função que imprime a informação sobre os comandos disponiveis
 * ao utilizador.
 */
void printHelp() {
    printf("Acoes disponiveis:\n\trequest client restaurant dish - criar um novo pedido\n");
    printf("\tstatus id - consultar o estado de um pedido \n");
    printf("\tstop - termina a execucao do magnaeats.\n");
    printf("\thelp - imprime informacao sobre as acoes disponiveis.\n");
    return;
}

/* Função que faz interação do utilizador, podendo receber 4 comandos:
 * request - cria uma nova operação, através da função create_request
 * status - verifica o estado de uma operação através da função read_status
 * stop - termina o execução do MAGNAEATS através da função stop_execution
 * help - imprime informação sobre os comandos disponiveis
 */
void user_interaction(struct communication_buffers *buffers, struct main_data *data, struct semaphores *sems) {
    char line[8];
    int counter = 0;
    printHelp();

    while (1) {
        printf("Introduzir acao:\n");
        size_t size = 100;
        char *buffer = create_dynamic_memory(size * sizeof(char));

        scanf("%1s", line);

        for (int i = strlen(line) - 1; i >= 0; i--) {
            ungetc(line[i], stdin);
        }

        fgets(buffer, 100, stdin);
        appendInstruction(args->log_filename, buffer);

        for (int i = strlen(buffer) - 1; i >= 0; i--) {
            ungetc(buffer[i], stdin);
        }

        destroy_dynamic_memory(buffer);

        scanf("%7s", line);

        if (strcmp(line, "request") == 0) {
            create_request(&counter, buffers, data, sems);
            sleep(1);
        } else if (strcmp(line, "status") == 0) {
            read_status(data, sems);
        } else if (strcmp(line, "stop") == 0) {
            stop_execution(data, buffers, sems);
            return;
        } else if (strcmp(line, "help") == 0) {
            printHelp();
        } else {
            printf("Acao nao reconhecida, insira 'help' para assistencia.\n");
        }
    }
}

/* Se o limite de operações ainda não tiver sido atingido, cria uma nova
 * operação identificada pelo valor atual de op_counter e com os dados passados em
 * argumento, escrevendo a mesma no buffer de memória partilhada entre main e restaurantes.
 * Imprime o id da operação e incrementa o contador de operações op_counter.
 */
void create_request(int *op_counter, struct communication_buffers *buffers, struct main_data *data,
                    struct semaphores *sems) {
    if (*op_counter >= data->max_ops) {
        printf("Numero maximo de operacoes atingido.\n");
        return;
    }

    char client[11];
    char restaurant[11];
    char dish[MAX_REQUESTED_DISH_SIZE];

    scanf("%10s", client);
    if (!isNumber(client)) {
        strcpy(dish, client);
        strcpy(client, "-1");
    } else {
        scanf("%10s", restaurant);
        if (!isNumber(restaurant)) {
            strcpy(dish, restaurant);
            strcpy(restaurant, "-1");
        } else {
            scanf("%99s", dish);
        }
    }

    struct operation *op;
    op = create_dynamic_memory(sizeof(struct operation));
    op->requested_dish = create_dynamic_memory(MAX_REQUESTED_DISH_SIZE * sizeof(char));

    op->id = *op_counter;
    op->status = 'I';
    markTime(&(op->start_time));
    strcpy(op->requested_dish, dish);

    if (!isNumber(client)) {
        op->requesting_client = -1;
        op->requested_rest = -1;
    } else {
        op->requesting_client = atoi(client);
        op->requested_rest = atoi(restaurant);
    }

    struct operation *results = data->results;

    semaphore_mutex_lock(sems->results_mutex);
    while (results < data->results + sizeof(data->results)) {
        if (!(results->status == 'I' || results->status == 'R' || results->status == 'D' || results->status == 'C')) {
            memcpy(results, op, sizeof(struct operation));
            break;
        }
        results++;
    }
    semaphore_mutex_unlock(sems->results_mutex);

    produce_begin(sems->main_rest);
    write_main_rest_buffer(buffers->main_rest, data->buffers_size, op);
    printf("O pedido #%d foi criado.\n", *op_counter);
    produce_end(sems->main_rest);

    (*op_counter)++;

    destroy_dynamic_memory(op);
}

/* Função que lê um id de operação do utilizador e verifica se a mesma
 * é valida. Em ;caso afirmativo,
 * imprime informação da mesma, nomeadamente o seu estado, o id do cliente
 * que fez o pedido, o id do restaurante requisitado, o nome do prato pedido
 * e os ids do restaurante, motorista, e cliente que a receberam e processaram.
 */
void read_status(struct main_data *data, struct semaphores *sems) {
    char c[11];
    int id;

    scanf("%s", c);
    if (isNumber(c)) {
        id = atoi(c);
    } else {
        printf("id de pedido fornecido e invalido!\n");

        for (int i = strlen(c) - 1; i >= 0; i--) {
            ungetc(c[i], stdin);
        }

        return;
    }

    struct operation *results = data->results;

    semaphore_mutex_lock(sems->results_mutex);
    while (results < data->results + sizeof(results)) {
        if (results->id == id &&
            (results->status == 'I' || results->status == 'D' || results->status == 'C' || results->status == 'R')) {
            printf("Pedido %d com estado %c requisitado pelo cliente %d ao restaurante %d ", id, results->status,
                   results->requesting_client, results->requested_rest);
            printf("com o prato %s, ", results->requested_dish);

            if (results->requested_rest != -1 && results->requesting_client != -1 && results->status != 'I') {
                printf("foi tratado pelo restaurante %d, ", results->receiving_rest);
                printf("encaminhado pelo motorista %d, ", results->receiving_driver);

                if (results->status == 'D') {
                    printf("mas ainda nao foi recebido no cliente!\n");
                } else {
                    printf("e enviado ao cliente %d!\n", results->receiving_client);
                }
                return;
            } else {
                printf("ainda nao foi recebido no restaurante!\n");
                return;
            }
        }
        results++;
    }
    semaphore_mutex_unlock(sems->results_mutex);

    printf("id de pedido fornecido e invalido!\n");
    for (int i = strlen(c) - 1; i >= 0; i--) {
        ungetc(c[i], stdin);
    }
    return;
}

/* Função que inicializa os semáforos da estrutura semaphores. Semáforos
 * *_full devem ser inicializados com valor 0, semáforos *_empty com valor
 * igual ao tamanho dos buffers de memória partilhada, e os *_mutex com
 * valor igual a 1. Para tal pode ser usada a função semaphore_create.*/
void create_semaphores(struct main_data *data, struct semaphores *sems) {
    sems->main_rest->full = semaphore_create(STR_SEM_MAIN_REST_FULL, 0);
    sems->main_rest->empty = semaphore_create(STR_SEM_MAIN_REST_EMPTY, data->buffers_size);
    sems->main_rest->mutex = semaphore_create(STR_SEM_MAIN_REST_MUTEX, 1);

    sems->rest_driv->full = semaphore_create(STR_SEM_REST_DRIV_FULL, 0);
    sems->rest_driv->empty = semaphore_create(STR_SEM_REST_DRIV_EMPTY, data->buffers_size);
    sems->rest_driv->mutex = semaphore_create(STR_SEM_REST_DRIV_MUTEX, 1);

    sems->driv_cli->full = semaphore_create(STR_SEM_DRIV_CLI_FULL, 0);
    sems->driv_cli->empty = semaphore_create(STR_SEM_DRIV_CLI_EMPTY, data->buffers_size);
    sems->driv_cli->mutex = semaphore_create(STR_SEM_DRIV_CLI_MUTEX, 1);

    sems->results_mutex = semaphore_create(STR_SEM_RESULTS_MUTEX, 1);
}

/* Função que acorda todos os processos adormecidos em semáforos, para que
 * estes percebam que foi dada ordem de terminação do programa. Para tal,
 * pode ser usada a função produce_end sobre todos os conjuntos de semáforos
 * onde possam estar processos adormecidos e um número de vezes igual ao
 * máximo de processos que possam lá estar.*/
void wakeup_processes(struct main_data *data, struct semaphores *sems) {
    int i;
    for (i = 0; i < data->max_ops; i++) {
        produce_end(sems->main_rest);
        produce_end(sems->driv_cli);
        produce_end(sems->rest_driv);
    }
}

/* Função que liberta todos os semáforos da estrutura semaphores. */
void destroy_semaphores(struct semaphores *sems) {
    semaphore_destroy(STR_SEM_MAIN_REST_FULL, sems->main_rest->full);
    semaphore_destroy(STR_SEM_MAIN_REST_EMPTY, sems->main_rest->empty);
    semaphore_destroy(STR_SEM_MAIN_REST_MUTEX, sems->main_rest->mutex);

    semaphore_destroy(STR_SEM_REST_DRIV_FULL, sems->rest_driv->full);
    semaphore_destroy(STR_SEM_REST_DRIV_EMPTY, sems->rest_driv->empty);
    semaphore_destroy(STR_SEM_REST_DRIV_MUTEX, sems->rest_driv->mutex);

    semaphore_destroy(STR_SEM_DRIV_CLI_FULL, sems->driv_cli->full);
    semaphore_destroy(STR_SEM_DRIV_CLI_EMPTY, sems->driv_cli->empty);
    semaphore_destroy(STR_SEM_DRIV_CLI_MUTEX, sems->driv_cli->mutex);

    semaphore_destroy(STR_SEM_RESULTS_MUTEX, sems->results_mutex);
}

void stop_execution_handler(int sig, siginfo_t *info, void *ucontext) {
    stop_execution(global_data, global_buffers, global_sems);
    exit(0);
}

/* Função que termina a execução do programa MAGNAEATS. Deve começar por
 * afetar a flag data->terminate com o valor 1. De seguida, e por esta
 * ordem, deve esperar que os processos filho terminem, deve escrever as
 * estatisticas finais do programa, e por fim libertar
 * as zonas de memória partilhada e dinâmica previamente
 * as zonas de memória partilhada e dinâmica previamente
 * reservadas. Para tal, pode usar as outras funções auxiliares do main.h.
 */
void stop_execution(struct main_data *data, struct communication_buffers *buffers, struct semaphores *sems) {
    *data->terminate = 1;
    wakeup_processes(data, sems);
    wait_processes(data);
    write_statistics(data);
    destroy_memory_buffers(data, buffers);
    destroy_semaphores(sems);
}
/* Função que espera que todos os processos previamente iniciados terminem,
 * incluindo restaurantes, motoristas e clientes. Para tal, pode usar a função
 * wait_process do process.h.
 */
void wait_processes(struct main_data *data) {
    int i = 0;

    for (i = 0; i < data->n_restaurants; i++) {
        data->restaurant_stats[i] = wait_process(data->restaurant_pids[i]);
    }

    for (i = 0; i < data->n_drivers; i++) {
        data->driver_stats[i] = wait_process(data->driver_pids[i]);
    }

    for (i = 0; i < data->n_clients; i++) {
        data->client_stats[i] = wait_process(data->client_pids[i]);
    }
}

/* Função que imprime as estatisticas finais do MAGNAEATS, nomeadamente quantas
 * operações foram processadas por cada restaurante, motorista e cliente.
 */
void write_statistics(struct main_data *data) {
    writeFile(args->statistics_filename, data);
    printf("Terminando o MAGNAEATS!\n");
}

/* Função que liberta todos os buffers de memória dinâmica e partilhada previamente
 * reservados na estrutura data.
 */
void destroy_memory_buffers(struct main_data *data, struct communication_buffers *buffers) {
    destroy_dynamic_memory(data->client_stats);
    destroy_dynamic_memory(data->driver_stats);
    destroy_dynamic_memory(data->restaurant_stats);
    destroy_dynamic_memory(data->client_pids);
    destroy_dynamic_memory(data->driver_pids);
    destroy_dynamic_memory(data->restaurant_pids);

    for (int i = 0; i < data->max_ops; i++) {
        destroy_dynamic_memory(data->results[i].requested_dish);
    }

    destroy_shared_memory(STR_SHM_MAIN_REST_BUFFER, buffers->main_rest->buffer,
                          data->buffers_size * sizeof(struct operation));
    destroy_shared_memory(STR_SHM_MAIN_REST_PTR, buffers->main_rest->ptrs, data->buffers_size * sizeof(int));

    destroy_shared_memory(STR_SHM_REST_DRIVER_BUFFER, buffers->rest_driv->buffer,
                          data->buffers_size * sizeof(struct operation));
    destroy_shared_memory(STR_SHM_REST_DRIVER_PTR, buffers->rest_driv->ptrs,
                          data->buffers_size * sizeof(struct pointers));

    destroy_shared_memory(STR_SHM_DRIVER_CLIENT_BUFFER, buffers->driv_cli->buffer,
                          data->buffers_size * sizeof(struct operation));
    destroy_shared_memory(STR_SHM_DRIVER_CLIENT_PTR, buffers->driv_cli->ptrs, data->buffers_size * sizeof(int));

    destroy_shared_memory(STR_SHM_RESULTS, data->results, data->n_clients * sizeof(struct operation));
    destroy_shared_memory(STR_SHM_TERMINATE, data->terminate, sizeof(int));
}

int main(int argc, char *argv[]) {
    // init data structures
    global_data = create_dynamic_memory(sizeof(struct main_data));
    global_buffers = create_dynamic_memory(sizeof(struct communication_buffers));
    global_buffers->main_rest = create_dynamic_memory(sizeof(struct rnd_access_buffer));
    global_buffers->rest_driv = create_dynamic_memory(sizeof(struct circular_buffer));
    global_buffers->driv_cli = create_dynamic_memory(sizeof(struct rnd_access_buffer));

    // init semaphore data structure
    global_sems = create_dynamic_memory(sizeof(struct semaphores));
    global_sems->main_rest = create_dynamic_memory(sizeof(struct prodcons));
    global_sems->rest_driv = create_dynamic_memory(sizeof(struct prodcons));
    global_sems->driv_cli = create_dynamic_memory(sizeof(struct prodcons));

    // execute main code
    main_args(argc, argv, global_data);
    create_dynamic_memory_buffers(global_data);
    create_shared_memory_buffers(global_data, global_buffers);
    create_semaphores(global_data, global_sems);
    launch_processes(global_buffers, global_data, global_sems);

    user_interaction(global_buffers, global_data, global_sems);

    // release memory before terminating
    destroy_dynamic_memory(global_data);
    destroy_dynamic_memory(global_buffers->main_rest);
    destroy_dynamic_memory(global_buffers->rest_driv);
    destroy_dynamic_memory(global_buffers->driv_cli);
    destroy_dynamic_memory(global_buffers);
    destroy_dynamic_memory(global_sems->main_rest);
    destroy_dynamic_memory(global_sems->rest_driv);
    destroy_dynamic_memory(global_sems->driv_cli);
    destroy_dynamic_memory(global_sems);
}
