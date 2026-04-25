#ifndef BIBLIOTECA
#define BIBLIOTECA

#define BUFFER_SIZE_ESCRITA 10
#define BUFFER_SIZE_VISUALIZAR 50

#include <time.h>

typedef struct {
    int numeroDoc;
    long int offsetCont;
} super;

typedef struct {
    char nome[256];
    long int tamanho;
    time_t data;
    long int offset;
} documento;

typedef struct {
    super *sup;
    documento *dir;
} biblioteca;

super *inicializarSuperbloco();

void gbvAdicionar(int tam, char *argv[], biblioteca *biblio, FILE *GBV);

void gbvRemover(int tam, char *argv[], biblioteca *biblio, FILE *GBV);

void gbvVisualizar(int tam, char *argv[], documento *dir, FILE *GBV);

void gbvListar(biblioteca *biblio);

void gbvOrdenar(char *argv[], biblioteca *biblio, FILE *GBV);

#endif