// authors:
//     Bruno Soares    fc57100
//     Renato Custódio fc56320
//     João Vedor      fc56311

#include "../include/client.h"
#include "../include/main.h"
#include "../include/memory.h"
#include "../include/synchronization.h"

#include <stdio.h>
#include <string.h>

/* Função principal de um Cliente. Deve executar um ciclo infinito onde em
 * cada iteração lê uma operação da main e data->terminate ainda for igual a 0,
 * processa-a e guarda o resultado no histórico de operações da Main.
 * Operações com id igual a -1 são ignoradas (op inválida) e se
 * data->terminate for igual a 1 é porque foi dada ordem de terminação do programa,
 * portanto deve-se fazer return do o número de operações processadas. Para efetuar
 * estes passos, pode usar os outros métodos auxiliares definidos em client.h.
 */
int execute_client(int client_id, struct communication_buffers* buffers, struct main_data* data, struct semaphores *sems) {
    int counter = 0;
    while (1) {
        struct operation* op;
        op = create_dynamic_memory(sizeof(struct operation));

        client_get_operation(op, client_id, buffers, data, sems);
        if (*data->terminate == 0) {
            if (op->id != -1) {
                client_process_operation(op, client_id, data, &counter, sems);
            }
        } else {
            destroy_dynamic_memory(op);
            return counter;
        }

        destroy_dynamic_memory(op);
    }
}

/* Função que lê uma operação do buffer de memória partilhada entre os motoristas
 * e clientes que seja direcionada a client_id. Antes de tentar ler a operação, deve
 * verificar se data->terminate tem valor 1. Em caso afirmativo, retorna imediatamente da função.
 */
void client_get_operation(struct operation* op, int client_id, struct communication_buffers* buffers, struct main_data* data, struct semaphores *sems) {
    if (*data->terminate != 1) {
        consume_begin(sems->driv_cli);
        read_driver_client_buffer(buffers->driv_cli, client_id, sizeof(buffers->main_rest), op);
        consume_end(sems->driv_cli);
    }
    return;
}

/* Função que processa uma operação, alterando o seu campo receiving_client para o id
 * passado como argumento, alterando o estado da mesma para 'C' (client), e
 * incrementando o contador de operações. Atualiza também a operação na estrutura data.
 */
void client_process_operation(struct operation* op, int client_id, struct main_data* data, int* counter, struct semaphores *sems) {
    printf("Cliente recebeu pedido!\n");
    op->receiving_client = client_id;
    op->status = 'C';

    semaphore_mutex_lock(sems->results_mutex);
    
    struct operation* results = data->results;
    while (results < data->results + sizeof(results)) {
        if (results->status == 'D') {
            memcpy(results, op, sizeof(struct operation));
            (*counter)++;
            break;
        }
        results++;
    }

    semaphore_mutex_unlock(sems->results_mutex);
    
    fflush(stdout);
}
