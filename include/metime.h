#ifndef METIME_H_GUARD
#define METIME_H_GUARD

#include <stdio.h>
#include <time.h>

#include "main.h"
#include "memory.h"

// converte um timespec para uma string e coloca-a num buffer com um formato especico,
// precedendo-a com str.
void tSpecToTm(char *buffer, time_t *timespec, char *str);

// A funcao clock_gettime() retorna o tempo atual pelo relogio especificado por clock_id,
// e coloca-o no buffer apontado por tp. A unica clock ID suportada e CLOCK_REALTIME.
void markTime(struct timespec* timeS);

#endif

