// authors:
//     Bruno Soares    fc57100
//     Renato Custódio fc56320
//     João Vedor      fc56311



#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../include/mesignal.h"
#include "../include/main-private.h"
#include "../include/main.h"

int alarmeLoop(struct main_data *data, int alarm_time) {
    while (1) {
        if (*data->terminate == 1) {
            return 0;
        }

        struct operation *res = data->results;

        while (res < data->results + sizeof(res)) {
            if (res->status == 'C')
                printf(
                    "request: %d status: %c start: %ld restaurant %d rest_time: %ld\ndriver: %d driver_time: %ld "
                    "client: %d client_end_time: %ld\n",
                    res->id, res->status, res->start_time.tv_sec, res->receiving_rest, res->rest_time.tv_sec,
                    res->receiving_driver, res->driver_time.tv_sec, res->receiving_client, res->client_end_time.tv_sec);
            else if (res->status == 'I' && res->status == 'R' && res->status == 'D') {
                printf("request: %d status: %d\n", res->id, res->status);
            }
            res++;
        }
        sleep(alarm_time);
    }
}

int launch_alarme(struct main_data *data, int alarm_time) {
    fflush(stdout);
    int pid = fork();

    if (pid == -1) {
        exit(1);
    } else if (pid == 0) {
        block_signal();
        exit(alarmeLoop(data, alarm_time));
    } else {
        return pid;
    }
}

int stop_signal() {
    struct sigaction sa;
    sigset_t block_mask;
    sigemptyset(&sa.sa_mask);
    sigemptyset(&block_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = stop_execution_handler;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    return 0;
}

int block_signal() {
    signal(SIGINT, SIG_IGN);
    return 0;
}
