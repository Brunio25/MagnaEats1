#ifndef CONFIGURATION_H_GUARD
#define CONFIGURATION_H_GUARD

//Estrutura que agrega a informação necessária para iniciar o programa.
struct args {
    int max_ops;                    //número máximo de operações
    int buffers_size;               //tamanho máximo dos buffers de mem. partilhada
    int n_restaurants;              //número de restaurantes
    int n_drivers;                  //número de motoristas
    int n_clients;                  //número de clientes

    char* log_filename;             //nome para o ficheiro de logs
    char* statistics_filename;      //nome para o ficheiro de estatisticas
    int alarm_time;                 //intervalo de tempo para um alarme
};

/*Funçao que recebe o nome de um ficheiro e um struct e mete
os valores lidos em cada linha numo campo indicado do struct
*/
int readFile(char* filename, struct args* dest);


#endif
