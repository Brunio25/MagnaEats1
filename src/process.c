// authors:
//     Bruno Soares    fc57100
//     Renato Custódio fc56320
//

#include "../include/process.h"
#include "../include/client.h"
#include "../include/driver.h"
#include "../include/main.h"
#include "../include/memory.h"
#include "../include/restaurant.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



/* Função que inicia um novo processo restaurante através da função fork do SO. O novo
 * processo irá executar a função execute_restaurant respetiva, fazendo exit do retorno.
 * O processo pai devolve o pid do processo criado.
 */
int launch_restaurant(int restaurant_id, struct communication_buffers* buffers, struct main_data* data) {
    fflush(stdout);
    int pid = fork();

    if (pid == -1) {
        exit(1);
    } else if (pid == 0) {
        exit(execute_restaurant(restaurant_id, buffers, data));
    } else {
        return pid;
    }
}

/* Função que inicia um novo processo motorista através da função fork do SO. O novo
 * processo irá executar a função execute_driver, fazendo exit do retorno.
 * O processo pai devolve o pid do processo criado.
 */
int launch_driver(int driver_id, struct communication_buffers* buffers, struct main_data* data) {
    fflush(stdout);
    int pid = fork();

    if (pid == -1) {
        exit(1);
    } else if (pid == 0) {
        exit(execute_driver(driver_id, buffers, data));
    } else {
        return pid;
    }
}

/* Função que inicia um novo processo cliente através da função fork do SO. O novo
 * processo irá executar a função execute_client, fazendo exit do retorno.
 * O processo pai devolve o pid do processo criado.
 */
int launch_client(int client_id, struct communication_buffers* buffers, struct main_data* data) {
    fflush(stdout);
    int pid = fork();
    
    if (pid == -1) {
        exit(1);
    } else if (pid == 0) {
        exit(execute_client(client_id, buffers, data));
    } else {
        return pid;
    }
}

/* Função que espera que um processo termine através da função waitpid.
 * Devolve o retorno do processo, se este tiver terminado normalmente.
 */
int wait_process(int process_id) {
    fflush(stdout);
    int res;

    waitpid(process_id, &res, WUNTRACED);
    if (WIFEXITED(res)) {
        return WEXITSTATUS(res);
    }

    exit(1);
}