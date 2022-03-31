#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

struct pointers
{
	int in;
	int out;
};

// estrutura que representa um buffer circular
struct circular_buffer
{
	struct pointers *ptrs;
	struct operation *buffer;
};

// estrutura que representa um buffer de acesso aleatório
struct rnd_access_buffer
{
	int *ptrs;
	struct operation *buffer;
};

// Estrutura que representa uma operação (pedido/resposta)
struct operation
{
	int id;				   // id da operação
	int requested_rest;	   // id do restaurante requisitado
	int requesting_client; // id do cliente que fez o pedido
	char *requested_dish;  // nome do(s) prato(s) pedido(s)

	char status;		  // estado da operação
	int receiving_rest;	  // id do restaurante que recebeu pedido
	int receiving_driver; // id do motorista que fez entrega
	int receiving_client; // id do cliente que recebeu a encomenda
};

// estrutura que agrega os shared memory buffers necessários para comunicação entre processos
struct communication_buffers
{
	struct rnd_access_buffer *main_rest; // buffer para main enviar pedidos a restaurantes
	struct circular_buffer *rest_driv;	 // buffer para restaurantes encaminharem pedidos a motoristas
	struct rnd_access_buffer *driv_cli;	 // buffer para motoristas entregarem pedidos aos clientes
};

/* Função que reserva uma zona de memória partilhada com tamanho indicado
 * por size e nome name, preenche essa zona de memória com o valor 0, e
 * retorna um apontador para a mesma. Pode concatenar o resultado da função
 * getuid() a name, para tornar o nome único para o processo.
 */
void *create_shared_memory(char *name, int size)
{
	int *ptr;
	int shm = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	int ret = ftruncate(shm, sizeof(int));

	if (ret == -1)
	{
		perror("/shm");
		exit(1);
	}

	ptr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
	if (ptr == MAP_FAILED)
	{
		perror("/shm-mmap");
		exit(2);
	}
}

/* Função que reserva uma zona de memória dinâmica com tamanho indicado
 * por size, preenche essa zona de memória com o valor 0, e retorna um
 * apontador para a mesma.
 */
void *create_dynamic_memory(int size)
{
	int n;
	int *buffer;
	buffer = calloc(size + 1, sizeof(int));
}

/* Função que liberta uma zona de memória dinâmica previamente reservada.
 */
void destroy_shared_memory(char *name, void *ptr, int size)
{
	int ret = munmap(ptr, sizeof(int));
	if (ret == -1)
	{
		perror("/shm");
		exit(3);
	}

	ret = shm_unlink("/shm");
	if (ret == -1)
	{
		perror("/shm");
		exit(4);
	}
}

/* Função que liberta uma zona de memória partilhada previamente reservada.
 */
void destroy_dynamic_memory(void *ptr)
{
	free(ptr);
}

/* Função que escreve uma operação no buffer de memória partilhada entre a Main
 * e os restaurantes. A operação deve ser escrita numa posição livre do buffer,
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_main_rest_buffer(struct rnd_access_buffer *buffer, int buffer_size, struct operation *op)
{
	for (int i = 0; i < buffer_size; i++)
	{
		if (buffer[i].ptrs == 0)
		{
			buffer[i].buffer = op;
			buffer[i].ptrs = 1;
			break;
		}
	}
}

/* Função que escreve uma operação no buffer de memória partilhada entre os restaurantes
 * e os motoristas. A operação deve ser escrita numa posição livre do buffer,
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_rest_driver_buffer(struct circular_buffer *buffer, int buffer_size, struct operation *op)
{
	/* produzir um item em next_produced */
	while (((buffer->ptrs->in + 1) % buffer_size) == buffer->ptrs->out);
	// buffer cheio; esperar que out seja avançado pelo consumidor
	/* do nothing */
	buffer[buffer->ptrs->in].buffer = op;
	buffer->ptrs->in = (buffer->ptrs->in + 1) % buffer_size;
}
	/* Função que escreve uma operação no buffer de memória partilhada entre os motoristas
	 * e os clientes. A operação deve ser escrita numa posição livre do buffer,
	 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
	 * Se não houver nenhuma posição livre, não escreve nada.
	 */
	void write_driver_client_buffer(struct rnd_access_buffer * buffer, int buffer_size, struct operation *op) {
		
	}

	/* Função que lê uma operação do buffer de memória partilhada entre a Main
	 * e os restaurantes, se houver alguma disponível para ler que seja direcionada ao restaurante especificado.
	 * A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
	 * Se não houver nenhuma operação disponível, afeta op->id com o valor -1.
	 */
	void read_main_rest_buffer(struct rnd_access_buffer * buffer, int rest_id, int buffer_size, struct operation *op) {
		int bool = 0;
		for (int i = 0; i < buffer_size && bool == 0; i++) {
			if (buffer[i].ptrs == 1 && buffer[i].buffer->requested_rest == rest_id)   //TODO check correct restaurant in buffer
			{
				op = buffer[i].buffer;
				buffer[i].ptrs = 0;
				bool = 1;
			}
		}

		if (bool == 0) {
			op->id = -1;
		}
	}

	/* Função que lê uma operação do buffer de memória partilhada entre os restaurantes e os motoristas,
	 * se houver alguma disponível para ler (qualquer motorista pode ler qualquer operação).
	 * A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
	 * Se não houver nenhuma operação disponível, afeta op->id com o valor -1.
	 */
	void read_rest_driver_buffer(struct circular_buffer * buffer, int buffer_size, struct operation *op) {
		int bool = 0;
		for (int i = 0; i < buffer_size && bool == 0; i++) {
			if (buffer[i].ptrs == 1)   //TODO check correct restaurant in buffer
			{
				op = buffer[i].buffer;
				buffer[i].ptrs = 0;
				bool = 1;
			}
		}

		if (bool == 0) {
			op->id = -1;
		}
	}

	/* Função que lê uma operação do buffer de memória partilhada entre os motoristas e os clientes,
	 * se houver alguma disponível para ler dirijida ao cliente especificado. A leitura deve
	 * ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo. Se não houver
	 * nenhuma operação disponível, afeta op->id com o valor -1.
	 */
	void read_driver_client_buffer(struct rnd_access_buffer * buffer, int client_id, int buffer_size, struct operation *op) {
		int bool = 0;
		for (int i = 0; i < buffer_size && bool == 0; i++) {
			if (buffer[i].ptrs == 1 && buffer[i].buffer->receiving_client == client_id)   //TODO check correct client in buffer
			{
				op = buffer[i].buffer;
				buffer[i].ptrs = 0;
				bool = 1;
			}
		}

		if (bool == 0) {
			op->id = -1;
		}
	}
