#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "../include/main.h"
#include "../include/main-private.h"
#include "../include/mesignal.h"

int alarmeLoop(struct main_data *data, int alarm_time) {
   while (1)
   {
        if (*data->terminate == 1) {
            return 0;
        }

        struct operation *res = data->results;

        while (res < data->results + sizeof(res)) {

            if (res->status == 'C')
                printf("request: %d status: %c start: %ld restaurant %d rest_time: %ld\ndriver: %d driver_time: %ld client: %d client_end_time: %ld\n",
                res->id, res->status, res->start_time.tv_sec, res-> receiving_rest, res->rest_time.tv_sec, res->receiving_driver, res->driver_time.tv_sec, 
                res->receiving_client, res->client_end_time.tv_sec);
            else if (res->status  == 'I' && res->status == 'R' && res->status == 'D') {
                printf("request: %d status: %d\n", res->id, res->status);
            }
            res++;
        }
        sleep(alarm_time);
   }
}

int launch_alarme(struct main_data* data, int alarm_time) {
    fflush(stdout);
    int pid = fork();

    if (pid == -1) {
        exit(1);
    } else if (pid == 0) {
        exit(alarmeLoop(data, alarm_time));
    } else {
        return pid;
    }
}

int stop_signal() {
    //sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
    struct sigaction sa;
    sa.sa_handler = stop_execution_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if(sigaction(SIGINT, &sa, NULL) == -1){
        perror("--");
        return -1;
    }

    return 0;
}


