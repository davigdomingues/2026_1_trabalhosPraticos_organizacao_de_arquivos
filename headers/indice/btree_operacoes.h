#ifndef BTREE_OPERACOES_H
#define BTREE_OPERACOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../dados/registro.h"
#include "../utils.h"
#include "./btree_no.h"

#define PROMOCAO 1
#define SEM_PROMOCAO 0
#define ERRO_DE_INSERCAO -1

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros);
bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados);
int insertIndice(FILE *fileIndice, int chave, int ponteiroDados);
int insertIndiceRec(FILE *fileIndice, int chave, int ponteiroDados, int rrnNoAtual, int *chavePromocao, int *filhoDirPromocao);
bool encontrarPos(No *no, int chave, int *pos, int *ponteiroDados);

#endif