#include "../include/configuration.h"
#include "../include/main-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

int readFile(char* filename, struct args* dest) {
    FILE *file;
    int args[6];

    struct stat buff;
    int exist = stat(filename, &buff);
    if (exist == -1) {
        return -1;
    }
    
    file = fopen(filename, "r");
    int n = 100;
    char token[n];
    // getline(&token, &n, file);
   
    int i = 0;
    while (fgets(token, n, file) != NULL)
    {
        printf("token: %s\n", token);
        if (i == 5)  {
            dest->log_filename = token;
            continue;
        }

        if (i == 6) {
            dest->statistics_filename = token;
            continue;
        }
        
        if (!isNumber(token)) {
            printf("Parametros incorretos!\nExistem parametros incorretos no ficheiro !\n");
            return -1;
        }


        args[i] = atoi(token);
        i++;
    }

    dest->max_ops = args[0];
    dest->buffers_size = args[1];
    dest->n_restaurants = args[2];
    dest->n_drivers = args[3];
    dest->n_clients = args[4];

    dest->alarm_time = args[5];
    
    fclose(file);
    
    return 0;
}