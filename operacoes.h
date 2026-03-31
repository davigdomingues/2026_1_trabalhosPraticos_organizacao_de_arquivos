#ifndef OPERACOES_H
#define OPERACOES_H

#include <stdio.h>
#include <stdbool.h>
#include "registro.h"

typedef struct CampoValor {
    char *campo;
    char *valor;
} CampoValor;

void create(char *arquivoEntrada, char *arquivoSaida);
void selectAll(char *arquivoEntrada);
Registro *selectWhere(char *arquivoEntrada, CampoValor *par, int mPares, int *tamResultados, int **rrns);
bool deleteWhere(char *arquivoEntrada, CampoValor *pares, int mPares);
bool insert(char *arquivoEntrada, CampoValor *valores, int mValores);
bool update(char *arquivoEntrada, char *arquivoSaida, CampoValor *paresBusca, int mParesBusca, CampoValor *paresUpdate, int mParesUpdate);

#endif