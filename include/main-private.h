#include <signal.h>

#ifndef MAX_REQUESTED_DISH_SIZE
#define MAX_REQUESTED_DISH_SIZE 100

/* Função que imprime a informação sobre os comandos disponiveis
* ao utilizador.
*/
void printHelp();

// Função que verifica se uma string é composta apenas por digitos
int isNumber(char* str);

void stop_execution_handler(int sig, siginfo_t *info, void *ucontext);

#endif