#include "../include/log.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "../include/metime.h"

// Retorna um long com os primeiros três dígitos de um long
long firstThree(long nSec) {
    long digits[4];
    int i;
    for (i = 0; i < 4; i++) {
        digits[i] = nSec;
    }

    while (nSec) {
        for (i = 4; i >= 1; i--) {
            digits[i] = digits[i - 1];
        }
        digits[0] = nSec;
        nSec /= 10;
    }

    return digits[2];
}

// Método que imprime as informações relativamente a um commando
// para um ficheiro filename.
void appendInstruction(char *filename, char *str) {
    FILE *file;
    file = fopen(filename, "a");

    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);

    char buffer[100];

    struct timespec *timeS = create_dynamic_memory(sizeof(struct timespec));

    markTime(timeS);

    char *sec = create_dynamic_memory(100);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", info);

    char decimal[10];
    sprintf(decimal, ".%lu", firstThree(timeS->tv_nsec));

    strcat(buffer, strcat(decimal, " "));
    strcat(buffer, str);

    fputs(buffer, file);

    destroy_dynamic_memory(timeS);
    destroy_dynamic_memory(sec);
    fclose(file);
}
