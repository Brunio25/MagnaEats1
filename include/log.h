#ifndef LOG_H_GUARD
#define LOG_H_GUARD

// Retorna um long com os primeiros três dígitos de um long
long firstThree(long nSec);

// Método que imprime as informacoes relativamente a um commando
// para um ficheiro filename.
void appendInstruction(char *filename, char *str);

#endif