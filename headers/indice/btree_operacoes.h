#ifndef BTREE_OPERACOES_H
#define BTREE_OPERACOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../dados/registro.h"
#include "../utils.h"

int selectWhereIndexado(FILE *fileDados, CampoValor *pares[8], int numFiltros);
bool buscaRecursiva(FILE *fileIndice, int chave, int rrn, int *rrnRes, int *ponteiroRes);

#endif