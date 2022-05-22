#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "../include/stats.h"
#include "../include/log.h"
#include "../include/metime.h"

double calcTotalTime (const struct timespec *endTime, const struct timespec *startTime) {
    return (endTime->tv_sec - startTime->tv_sec) + (endTime->tv_nsec - startTime->tv_nsec) / 1000000000.0;
}

void writeFile(const char* filename, struct main_data *data) {
    struct operation *op = data -> results;
    FILE *file;
    file = fopen(filename, "a");
    char buffer[50];

    snprintf(buffer, 50,"Process Statistics:\n");
    fputs(buffer, file);

    for (int i = 0; i < data->n_restaurants; i++)
    {
        snprintf(buffer, 50,"\t\tRestaurant %d prepared %d requests!\n", i, data->restaurant_stats[i]);
        fputs(buffer, file);
    }

    for (int i = 0; i < data->n_drivers; i++)
    {
        snprintf(buffer, 50,"\t\tDriver %d delivered %d requests!\n", i, data->driver_stats[i]);
        fputs(buffer, file);
    }

    for (int i = 0; i < data->n_clients; i++)
    {
        snprintf(buffer, 50,"\t\tClient %d received %d requests!\n", i, data->client_stats[i]);
        fputs(buffer, file);
    }
    
    snprintf(buffer, 50,"\nRequest Statistics:");
    fputs(buffer, file);
    while (op < data->results + sizeof(op) &&
    (op->status == 'I' || op->status == 'C' || op->status == 'D' || op->status == 'R')) {
        
        snprintf(buffer, 50,"\nRequest: %d\nStatus: %c\n", op->id, op->status);
        fputs(buffer, file);
        
        if (op->status != 'I') {
            snprintf(buffer, 50,"Restaurant id: %d\nDriver id: %d\nClient id: %d\n", op->receiving_rest, op->receiving_client, op->receiving_driver);
            fputs(buffer, file);
        }  

        tSpecToTm(buffer, &op->start_time.tv_nsec, "Created");
        fputs(buffer, file);
        
        if(op->status != 'I') {
            tSpecToTm(buffer, &op->rest_time.tv_nsec, "Restaurant time");
            fputs(buffer, file);

            tSpecToTm(buffer, &op->driver_time.tv_nsec, "Driver time");
            fputs(buffer, file);

            tSpecToTm(buffer, &op->client_end_time.tv_nsec, "Client time (end)");
            fputs(buffer, file);

            snprintf(buffer, 50, "Total Time: %f\n", calcTotalTime(&op->client_end_time, &op->start_time));
            fputs(buffer, file);
        }
        op++;
    }
}
