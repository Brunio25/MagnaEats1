#ifndef CONFIGURATION_H_GUARD
#define CONFIGURATION_H_GUARD
struct args {
    int max_ops;
    int buffers_size;
    int n_restaurants;
    int n_drivers;
    int n_clients;

    char* log_filename;
    char* statistics_filename;
    int alarm_time;
};

int readFile(char* filename, struct args* dest);


#endif
