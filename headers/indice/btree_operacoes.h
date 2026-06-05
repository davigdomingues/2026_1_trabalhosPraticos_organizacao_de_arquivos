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

/**
 * @brief cria a árvore-B de índice a partir do arquivo binário de dados.
 * 
 * @param arquivoDados caminho do arquivo de dados de entrada
 * @param arquivoIndice caminho do arquivo de índice de saída
 * @return true índice criado com sucesso
 * @return false falha no processamento
 */
bool criarIndiceArvoreB(char *arquivoDados, char *arquivoIndice);

#endif