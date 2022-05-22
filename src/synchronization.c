#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>  

#include "../include/synchronization.h"

/* Função que cria um novo semáforo com nome name e valor inicial igual a
 * value. Pode concatenar o resultado da função getuid() a name, para tornar
 * o nome único para o processo.
 */
sem_t *semaphore_create(char *name, int value)
{
    sem_t *sem;
    sem = sem_open(name, O_CREAT, 0xFFFFFFFF, value);
    if (sem == SEM_FAILED)
    {
        perror("sem_create");
        exit(1);
    }

    return sem;
}

/* Função que destroi o semáforo passado em argumento.
 */
void semaphore_destroy(char *name, sem_t *semaphore)
{

    if (sem_close(semaphore) == -1)
    {
        perror("sem_destroy");
        exit(1);
    }
    if (sem_unlink(name) == -1)
    {
        perror("sem_destroy");
        exit(1);
    }
}

/* Função que inicia o processo de produzir, fazendo sem_wait nos semáforos
 * corretos da estrutura passada em argumento.
 */
void produce_begin(struct prodcons *pc)
{
    sem_wait(pc -> empty);
    sem_wait(pc -> mutex);
}

/* Função que termina o processo de produzir, fazendo sem_post nos semáforos
 * corretos da estrutura passada em argumento.
 */
void produce_end(struct prodcons *pc)
{
    sem_post(pc -> mutex);
    sem_post(pc -> full);
}

/* Função que inicia o processo de consumir, fazendo sem_wait nos semáforos
 * corretos da estrutura passada em argumento.
 */
void consume_begin(struct prodcons *pc)
{
    sem_wait(pc -> full);
    sem_wait(pc -> mutex);

}

/* Função que termina o processo de consumir, fazendo sem_post nos semáforos
 * corretos da estrutura passada em argumento.
 */
void consume_end(struct prodcons *pc)
{
    sem_post(pc -> mutex);
    sem_post(pc -> empty);
}

/* Função que faz wait a um semáforo.
 */
void semaphore_mutex_lock(sem_t *sem)
{
    sem_wait(sem);
}

/* Função que faz post a um semáforo.
 */
void semaphore_mutex_unlock(sem_t *sem)
{
    sem_post(sem);
}