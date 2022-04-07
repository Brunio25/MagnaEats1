#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> 
#include "../include/main.h"
#include "../include/process.h"

/* Função que lê os argumentos da aplicação, nomeadamente o número
 * máximo de operações, o tamanho dos buffers de memória partilhada
 * usados para comunicação, e o número de clientes, de motoristas e de
 * restaurantes. Guarda esta informação nos campos apropriados da
 * estrutura main_data.
 */
void main_args(int argc, char* argv[], struct main_data* data) {
    data->max_ops = atoi(argv[1]);
    data->buffers_size = atoi(argv[2]);
    data->n_clients = atoi(argv[3]);
    data->n_restaurants = atoi(argv[4]);
    data->n_drivers = atoi(argv[5]);
}

/* Função que reserva a memória dinâmica necessária para a execução
* do MAGNAEATS, nomeadamente para os arrays *_pids e *_stats da estrutura 
* main_data. Para tal, pode ser usada a função create_dynamic_memory.
*/
void create_dynamic_memory_buffers(struct main_data* data) {
	data -> client_stats = create_dynamic_memory(data -> n_clients * sizeof(int));
	data -> driver_stats = create_dynamic_memory(data -> n_drivers * sizeof(int));
	data -> restaurant_stats = create_dynamic_memory(data -> n_restaurants * sizeof(int));
	data -> client_pids = create_dynamic_memory(data -> n_clients * sizeof(int));
	data -> driver_pids = create_dynamic_memory(data -> n_drivers * sizeof(int));
	data -> restaurant_pids = create_dynamic_memory(data -> n_restaurants * sizeof(int));
}

/* Função que reserva a memória partilhada necessária para a execução do
* MAGNAEATS. É necessário reservar memória partilhada para todos os buffers da
* estrutura communication_buffers, incluindo os buffers em si e respetivos
* pointers, assim como para o array data->results e variável data->terminate.
* Para tal, pode ser usada a função create_shared_memory.
*/
void create_shared_memory_buffers(struct main_data* data, struct communication_buffers* buffers) {
	buffers->main_rest->buffer = create_shared_memory(STR_SHM_MAIN_REST_BUFFER , data->buffers_size * sizeof(struct operation));
	buffers->main_rest->ptrs = create_shared_memory(STR_SHM_MAIN_REST_PTR , data->buffers_size * sizeof(int));

	buffers->rest_driv->buffer = create_shared_memory(STR_SHM_REST_DRIVER_BUFFER , data->buffers_size * sizeof(struct operation));
	buffers->rest_driv->ptrs = create_shared_memory(STR_SHM_REST_DRIVER_PTR , data->buffers_size * sizeof(struct pointers));
	
	buffers->driv_cli->buffer = create_shared_memory(STR_SHM_DRIVER_CLIENT_BUFFER, data->buffers_size * sizeof(struct operation));
	buffers->driv_cli->ptrs = create_shared_memory(STR_SHM_DRIVER_CLIENT_PTR, data->buffers_size * sizeof(int));
	
	data -> results = create_shared_memory(STR_SHM_RESULTS, data -> max_ops * sizeof(struct operation));
	data -> terminate = create_shared_memory(STR_SHM_TERMINATE, sizeof(int));
}

/* Função que inicia os processos dos restaurantes, motoristas e
* clientes. Para tal, pode usar as funções launch_*,
* guardando os pids resultantes nos arrays respetivos
* da estrutura data.
*/
void launch_processes(struct communication_buffers* buffers, struct main_data* data) {
	int i;
	for (i = 0; i < data->n_restaurants; i++) {
		data->restaurant_pids[i] = launch_restaurant(i, buffers, data);
	}
	
	for (i = 0; i < data->n_drivers; i++) {
		data->driver_pids[i] = launch_driver(i, buffers, data);
	}
	
	for (i = 0; i < data->n_clients; i++) {
		data->client_pids[i] = launch_client(i, buffers, data);
	}
	
}

/* Função que imprime a informação sobre os comandos disponiveis
* ao utilizador.
*/
void printHelp() {
	printf("Ações disponíveis:\n\trequest client restaurant dish - criar um novo pedido\n");
    printf("\tstatus id - consultar o estado de um pedido \n");
    printf("\tstop - termina a execução do magnaeats.\n");
    printf("\thelp - imprime informação sobre as ações disponíveis.\n");
	return;
}

/* Função que faz interação do utilizador, podendo receber 4 comandos:
* request - cria uma nova operação, através da função create_request
* status - verifica o estado de uma operação através da função read_status
* stop - termina o execução do MAGNAEATS através da função stop_execution
* help - imprime informação sobre os comandos disponiveis
*/
void user_interaction(struct communication_buffers* buffers, struct main_data* data) {
	char line[8];
	int counter = 0;
	printHelp();
	while (1) {
		printf("Introduzir ação:\n");
		scanf("%7s",line);

		if(strcmp(line, "request") == 0) {
			create_request(&counter, buffers, data);
			sleep(0.100);
		}
		else if (strcmp(line, "status") == 0) {
			read_status(data);
		}
		else if (strcmp(line, "stop") == 0) {
			stop_execution(data, buffers);
			return;
		}
		else if (strcmp(line, "help") == 0) {
			printHelp();
		} 
		else {
			printf("Ação não reconhecida, insira 'help' para assistência.\n");
		}
	}
}

// Função que verifica se uma string é composta apenas por digitos
int isNumber(char* str) {
	for (int i = 0; i < strlen(str); i++) {
		if (!isdigit(str[i])) {
			return 0;
		}
	}
	return 1;
}

/* Se o limite de operações ainda não tiver sido atingido, cria uma nova
* operação identificada pelo valor atual de op_counter e com os dados passados em
* argumento, escrevendo a mesma no buffer de memória partilhada entre main e restaurantes.
* Imprime o id da operação e incrementa o contador de operações op_counter.
*/
void create_request(int* op_counter, struct communication_buffers* buffers, struct main_data* data) {
	if (*op_counter > data->max_ops) {
		printf("Número máximo de operações atingido.\n");
		return;
	}
	char client[11];
	char restaurant[11];
	char dish[MAX_REQUESTED_DISH_SIZE];

	scanf("%10s", client);
	if (!isNumber(client)) {
		*dish = *client;
	}
	else {
		scanf("%10s", restaurant);
		if (!isNumber(restaurant)) {
			*dish = *restaurant;
			*restaurant = *"-1";
		}
		else {
			scanf("%99s", dish);
		}
	}
	
	struct operation *op;
	op = create_dynamic_memory(sizeof(struct operation));
	//op->requested_dish = create_dynamic_memory(MAX_REQUESTED_DISH_SIZE * sizeof(char));
	if (!isNumber(client)) {
		op->requesting_client = -1;
		op->requested_rest = -1;
	}
	else {
		op->requesting_client = atoi(client);
		op->requested_rest = atoi(restaurant); 
	}
	op->id = *op_counter;
	op->status = 'I';
	//strcpy(op->requested_dish, dish);

	write_main_rest_buffer(buffers->main_rest, data->buffers_size, op);
	printf("O pedido #%d foi criado.\n", *op_counter);

	destroy_dynamic_memory(op->requested_dish);
	destroy_dynamic_memory(op);

	(*op_counter)++;
}

/* Função que lê um id de operação do utilizador e verifica se a mesma
* é valida. Em ;caso afirmativo,
* imprime informação da mesma, nomeadamente o seu estado, o id do cliente
* que fez o pedido, o id do restaurante requisitado, o nome do prato pedido
* e os ids do restaurante, motorista, e cliente que a receberam e processaram.
*/
void read_status(struct main_data* data) {
	char c[11];
	int id;

	scanf("%s", c);
	if (isNumber(c)) {
		id = atoi(c);
	}
	else {
		printf("id de pedido fornecido é inválido!\n");
		
		for (int i = strlen(c) - 1; i >= 0; i--) {
			ungetc(c[i], stdin);
		}

		return;
	}
	
	struct operation* results = data->results;
	while (results < data->results + sizeof(results)) {
		//printf("id: %d dish: %s\n", results->id, results->requested_dish);
			if (results->id == id) { //TODO does not work in Invalid status operation
				if(results->requested_rest != -1 && results->requesting_client != -1) {
					printf("Pedido %d com estado %c requisitado pelo cliente %d ao restaurante %d ", id, results->status, results->requesting_client,results->requested_rest);
					printf("com o prato %s, foi tratado pelo restaurante %d,", results->requested_dish, results->receiving_rest);
					printf(" encaminhado pelo motorista %d, e enviado ao cliente %d!\n", results->receiving_driver, results->receiving_client);
					return;
				}
				else {
					printf("Pedido %d com estado %c requisitado pelo cliente %d ao restaurante %d ", id, results->status, results->requesting_client,results->requested_rest);
					printf("com o prato %s, ainda não foi recebido no restaurante!\n",results->requested_dish);
					return;
				}
			}
		results++;
	}

	printf("id de pedido fornecido é inválido!\n");
	for (int i = strlen(c) - 1; i >= 0; i--) {
			ungetc(c[i], stdin);
		}
	return;
}

/* Função que termina a execução do programa MAGNAEATS. Deve começar por 
* afetar a flag data->terminate com o valor 1. De seguida, e por esta
* ordem, deve esperar que os processos filho terminem, deve escrever as
* estatisticas finais do programa, e por fim libertar
* as zonas de memória partilhada e dinâmica previamente 
* as zonas de memória partilhada e dinâmica previamente 
* reservadas. Para tal, pode usar as outras funções auxiliares do main.h.
*/
void stop_execution(struct main_data* data, struct communication_buffers* buffers) {
	*data->terminate = 1;
	wait_processes(data);
	write_statistics(data);
	destroy_memory_buffers(data, buffers);
}
/* Função que espera que todos os processos previamente iniciados terminem,
* incluindo restaurantes, motoristas e clientes. Para tal, pode usar a função 
* wait_process do process.h.
*/
void wait_processes(struct main_data* data) {
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
void write_statistics(struct main_data* data) {
	printf("Terminando o MAGNAEATS! Imprimindo estatísticas:\n");
	for(int i = 0; i < data->n_restaurants; i++){
		printf("Restaurante %d preparou %d pedidos!\n", i, data->restaurant_stats[i]);
	}
	
	for (int i = 0; i < data->n_drivers; i++) {
		printf("Motorista %d entregou %d pedidos!\n", i, data->driver_stats[i]);
	}

	for(int i = 0; i < data->n_clients; i++) {
		printf("Cliente %d recebeu %d pedidos!\n",i , data->client_stats[i]);
	}
	
}

/* Função que liberta todos os buffers de memória dinâmica e partilhada previamente
* reservados na estrutura data.
*/
void destroy_memory_buffers(struct main_data* data, struct communication_buffers* buffers) {
	destroy_dynamic_memory(data -> client_stats);
	destroy_dynamic_memory(data -> driver_stats);
	destroy_dynamic_memory(data -> restaurant_stats);
	destroy_dynamic_memory(data -> client_pids);
	destroy_dynamic_memory(data -> driver_pids);
	destroy_dynamic_memory(data -> restaurant_pids);
	
	destroy_shared_memory("/main_rest_buffer", buffers->main_rest->buffer,data->buffers_size * sizeof(struct operation));
	destroy_shared_memory("/main_rest_ptrs", buffers->main_rest->ptrs,data->buffers_size * sizeof(int));
	destroy_shared_memory("/main_rest", buffers->main_rest, data -> buffers_size * sizeof(struct rnd_access_buffer));

	destroy_shared_memory("/rest_driv_buffer", buffers->rest_driv->buffer,data->buffers_size * sizeof(struct operation));
	destroy_shared_memory("/rest_driv_ptrs", buffers->rest_driv->ptrs,data->buffers_size * sizeof(struct pointers));
	destroy_shared_memory("/rest_driv", buffers->rest_driv, data -> buffers_size * sizeof(struct circular_buffer));

	destroy_shared_memory("/driv_cli_buffer", buffers->driv_cli->buffer, data->buffers_size * sizeof(struct operation));
	destroy_shared_memory("/driv_cli_ptrs", buffers->driv_cli->ptrs, data->buffers_size * sizeof(int));
	destroy_shared_memory("/driv_cli", buffers->driv_cli, data -> buffers_size * sizeof(struct rnd_access_buffer));

	destroy_shared_memory("/results", data -> results, data -> n_clients * sizeof(struct operation));
	destroy_shared_memory("/terminate", data -> terminate, sizeof(int));

	buffers->driv_cli->buffer = create_shared_memory("/driv_cli_buffer", data->buffers_size * sizeof(struct operation));
	buffers->driv_cli->buffer = create_shared_memory("/driv_cli_ptrs", data->buffers_size * sizeof(int));
	
	
}

int main(int argc, char *argv[]) {
	//init data structures
	struct main_data* data = create_dynamic_memory(sizeof(struct main_data));
	struct communication_buffers* buffers = create_dynamic_memory(sizeof(struct communication_buffers));
	buffers->main_rest = create_dynamic_memory(sizeof(struct rnd_access_buffer));
	buffers->rest_driv = create_dynamic_memory(sizeof(struct circular_buffer));
	buffers->driv_cli = create_dynamic_memory(sizeof(struct rnd_access_buffer));
	
	//execute main code

	main_args(argc, argv, data);
	create_dynamic_memory_buffers(data);
	create_shared_memory_buffers(data, buffers);
	launch_processes(buffers, data);
	user_interaction(buffers, data);

	//release memory before terminating

	destroy_dynamic_memory(data);
	destroy_dynamic_memory(buffers->main_rest);
	destroy_dynamic_memory(buffers->rest_driv);
	destroy_dynamic_memory(buffers->driv_cli);
	destroy_dynamic_memory(buffers);
}
