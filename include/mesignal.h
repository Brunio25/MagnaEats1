#ifndef MESIGNAL_H_GUARD
#define MESIGNAL_H_GUARD

#include "../include/main.h"

/*
O metodo vai criar um loop em que de x em x segundos, sendo que este valor x
e definido no parametro alarm_time, escreve no stdout informacoes sobre as 
operacoes existentes em memoria
*/
int alarmeLoop(struct main_data *data, int alarm_time);

/*
Funcao que cria um processo para o alarme possa funcionar 
*/
int launch_alarme(struct main_data* data, int alarm_time);

/*
Metodo que caso haja uma interrupcao do processo, envia um sinal para 
que um handler prossiga com um protocolo
*/
int stop_signal();

/*
Funcao que envia um sinal para bloquear sinais SIGINT (Interrupcoes do processo)
*/
int block_signal();

#endif