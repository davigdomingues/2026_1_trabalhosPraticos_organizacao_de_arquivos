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
// void insert(char *arquivoEntrada, char *arquivoSaida, CampoValor *pares, int mPares);
// void update(char *arquivoEntrada, char *arquivoSaida, CampoValor *paresBusca, int mParesBusca, CampoValor *paresUpdate, int mParesUpdate);