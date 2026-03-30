#include <stdio.h>
#include <stdbool.h>

typedef struct CampoValor {
    char *campo;
    char *valor;
} CampoValor;

void create(char *arquivoEntrada, char *arquivoSaida);
void selectAll(char *arquivoEntrada);
void selectWhere(char *arquivoEntrada, CampoValor *par, int mPares);
bool deleteWhere(char *arquivoEntrada, CampoValor *pares, int mPares);