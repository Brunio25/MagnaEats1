#include "../include/main.h"

#ifndef STATS_H_GUARD
#define STATS_H_GUARD

/* Funcao que recebe o nome de um ficheiro e uma struct com dados sobre o programa
e escreve no ficheiro as estaticticas desse mesmo programa
*/
void writeFile(const char* filename, struct main_data *data); 

/* Funcao que recebe um tempo final e um tempo inicial no formato struct timespec 
e calcula a diferenca entres os mesmos
*/
double calcTotalTime (const struct timespec *endTime, const struct timespec *startTime);

#endif