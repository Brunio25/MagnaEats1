#include "../include/memory.h"
#include "../include/main.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

/* Função que reserva uma zona de memória partilhada com tamanho indicado
 * por size e nome name, preenche essa zona de memória com o valor 0, e
 * retorna um apontador para a mesma. Pode concatenar o resultado da função
 * getuid() a name, para tornar o nome único para o processo.
 */
void *create_shared_memory(char *name, int size) {
    int *ptr;
    
    int shm = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    int ret = ftruncate(shm, size);

    if (ret == -1) {
        perror("shm");
        exit(1);
    }

    ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (ptr == MAP_FAILED) {
        perror("shm-mmap");
        exit(2);
    }
    memset(ptr, 0, size);
    return ptr;
}

/* Função que reserva uma zona de memória dinâmica com tamanho indicado
 * por size, preenche essa zona de memória com o valor 0, e retorna um
 * apontador para a mesma.
 */
void *create_dynamic_memory(int size) { return calloc(size + 1, sizeof(int)); }

/* Função que liberta uma zona de memória dinâmica previamente reservada.
 */
void destroy_shared_memory(char *name, void *ptr, int size) {
    int ret = munmap(ptr, size);
    if (ret == -1) {
        perror("/shm");
        exit(3);
    }

    ret = shm_unlink("/shm");
    if (ret == -1) {
        perror("/shm");
        exit(4);
    }
}

/* Função que liberta uma zona de memória partilhada previamente reservada.
 */
void destroy_dynamic_memory(void *ptr) { free(ptr); }

/* Função que escreve uma operação no buffer de memória partilhada entre a Main
 * e os restaurantes. A operação deve ser escrita numa posição livre do buffer,
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_main_rest_buffer(struct rnd_access_buffer *buffer, int buffer_size, struct operation *op) {
    for (int i = 0; i < buffer_size; i++) {
        if (buffer->ptrs[i] == 0) {
            
            memcpy(&buffer->buffer[i], op, sizeof(struct operation));
            printf("write1: %p\n", buffer->buffer[i].requested_dish);
            strcpy(buffer->buffer[i].requested_dish, op->requested_dish);
            //memcpy(&buffer->buffer[i].requested_dish, &op->requested_dish, (MAX_REQUESTED_DISH_SIZE+1) * sizeof(char));
            printf("write2: %p\n", buffer->buffer[i].requested_dish);
            printf("dish: %s\n", buffer->buffer[i].requested_dish);
            buffer->ptrs[i] = 1;
            break;
        } 
    }
}

/* Função que escreve uma operação no buffer de memória partilhada entre os restaurantes
 * e os motoristas. A operação deve ser escrita numa posição livre do buffer,
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_rest_driver_buffer(struct circular_buffer *buffer, int buffer_size, struct operation *op) {
    /* produzir um item em next_produced */
    while (((buffer->ptrs->in + 1) % buffer_size) == buffer->ptrs->out);

    // buffer cheio; esperar que out seja avançado pelo consumidor
    /* do nothing */
    //nos

    memcpy(&(buffer->buffer[buffer->ptrs->in]), op, sizeof(struct operation));
    buffer->ptrs->in = (buffer->ptrs->in + 1) % buffer_size;
}
/* Função que escreve uma operação no buffer de memória partilhada entre os motoristas
 * e os clientes. A operação deve ser escrita numa posição livre do buffer,
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_driver_client_buffer(struct rnd_access_buffer *buffer, int buffer_size, struct operation *op) {
    for (int i = 0; i < buffer_size; i++) {
        if (buffer->ptrs[i] == 0) {
            memcpy(&buffer->buffer[i], op, sizeof(struct operation));
            buffer->ptrs[i] = 1;
            break;
        }
    }
}

/* Função que lê uma operação do buffer de memória partilhada entre a Main
 * e os restaurantes, se houver alguma disponível para ler que seja direcionada ao restaurante especificado.
 * A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
 * Se não houver nenhuma operação disponível, afeta op->id com o valor -1.
 */
void read_main_rest_buffer(struct rnd_access_buffer *buffer, int rest_id, int buffer_size, struct operation *op) {
    int bool = 0;
    // printf("ptr: %d, rest: %d\n", buffer->ptrs[0], buffer->buffer[0].requested_rest);
    for (int i = 0; i < buffer_size && bool == 0; i++) {
        if (buffer->ptrs[i] == 1 &&
            buffer->buffer[i].requested_rest == rest_id) {
            printf("boas\n");
            printf("read: %p\n", buffer->buffer[i].requested_dish);
            printf("dish no read: %s\n", buffer->buffer[i].requested_dish);
            printf("rest: %d\n",buffer->buffer[i].requested_rest);
            strcpy(op->requested_dish, buffer->buffer[i].requested_dish);
            memcpy(op, &(buffer->buffer[i]), sizeof(struct operation));
            buffer->ptrs[i] = 0;
            bool = 1;
        }
    }

    if (bool == 0) {
        op->id = -1;
    }
    return;
}

/* Função que lê uma operação do buffer de memória partilhada entre os restaurantes e os motoristas,
 * se houver alguma disponível para ler (qualquer motorista pode ler qualquer operação).
 * A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
 * Se não houver nenhuma operação disponível, afeta op->id com o valor -1.
 */
void read_rest_driver_buffer(struct circular_buffer *buffer, int buffer_size, struct operation *op) {
    if (buffer->ptrs->in == buffer->ptrs->out) {
        op->id = -1;
        return;
    }
    
    int out = buffer->ptrs->out;

    memcpy(op, &(buffer->buffer[out]), sizeof(struct operation));
    memset(&(buffer->buffer[out]), -1, sizeof(struct operation));
    buffer->ptrs->out = (out + 1) % buffer_size;
}

/* Função que lê uma operação do buffer de memória partilhada entre os motoristas e os clientes,
 * se houver alguma disponível para ler dirijida ao cliente especificado. A leitura deve
 * ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo. Se não houver
 * nenhuma operação disponível, afeta op->id com o valor -1.
 */
void read_driver_client_buffer(struct rnd_access_buffer *buffer, int client_id, int buffer_size, struct operation *op) {
    int bool = 0;
    for (int i = 0; i < buffer_size && bool == 0; i++) {
        if (buffer->ptrs[i] == 1 &&
            buffer->buffer[i].requesting_client == client_id) {
            strcpy(op->requested_dish, buffer->buffer[i].requested_dish);
            memcpy(op, &(buffer->buffer[i]), sizeof(struct operation));
            buffer->ptrs[i] = 0;
            bool = 1;
        }
    }

    if (bool == 0) {
        op->id = -1;
    }
    return;
}
