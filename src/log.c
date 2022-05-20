#include "../include/log.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

void appendInstruction(char *filename, char *str) {
    FILE *file;
    file = fopen(filename, "a");

    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);

    char buffer[26];
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", info);

    char* cat = strcat(buffer," ");
    cat = strcat(cat, str);


    fputs(cat, file);

    fclose(file);
}
 