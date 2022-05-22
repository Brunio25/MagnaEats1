// authors:
//     Bruno Soares    fc57100
//     Renato Custódio fc56320
//     João Vedor      fc56311



#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/metime.h"
#include "../include/log.h"

// converte um timespec para uma string e coloca-a num buffer com um formato especico,
// precedendo-a com str.
void tSpecToTm(char *buffer, time_t *timespec, char *str) {
    struct tm *time;
    char timeBuff[26];
    time = localtime(timespec);
    strftime(timeBuff, 26, "%Y-%m-%d %H:%M:%S", time);
    snprintf(buffer, 50, "%s: %s.%ld\n", str, timeBuff, firstThree(*timespec));
}

// A funcao clock_gettime() retorna o tempo atual pelo relogio especificado por clock_id,
// e coloca-o no buffer apontado por tp. A unica clock ID suportada e CLOCK_REALTIME.
void markTime(struct timespec *timeS) {
    if (clock_gettime(CLOCK_REALTIME, timeS) == -1) {
        perror("clock error");
        exit(1);
    }
}
