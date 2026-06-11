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
#define MIN_CHAVES 1
#define SUCESSO 1
#define CHAVE_NAO_ENCONTRADA 0
#define UNDERFLOW_PENDENTE 2

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros, bool print);
bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados);
int insertIndice(FILE *fileIndice, int chave, int ponteiroDados);
int insertIndiceRec(FILE *fileIndice, int chave, int ponteiroDados, int rrnNoAtual, int *chaveASerPromovida, int *ponteiroDadosChaveASerPromovida, int *filhoDirChaveASerPromovida);

/**
 * @brief cria a árvore-B de índice a partir do arquivo binário de dados.
 * 
 * @param arquivoDados caminho do arquivo de dados de entrada
 * @param arquivoIndice caminho do arquivo de índice de saída
 * @return true índice criado com sucesso
 * @return false falha no processamento
 */
bool criarIndiceArvoreB(char *arquivoDados, char *arquivoIndice);

bool removerChaveIndice(FILE *fileIndice, int chave);
int removerRecursivo(FILE *fileIndice, int rrnAtual, int chave);
void tratarUnderflow(FILE *fileIndice, No *pai, int rrnPai, int indicePonteiroFilho);
void fazerMerge(FILE *fileIndice, No *esq, No *dir, No *pai, int rrnEsq, int rrnDir, int rrnPai, int indiceChavePai);

/**
 * @brief Remove registros do arquivo de dados e suas chaves da Árvore-B
 * dependendo se a chave primária foi fornecida na query (O(log n)) ou não (O(n)).
 */
bool deleteWhereIndexado(char *arquivoEntrada, char *arquivoIndice, CampoValor *pares, int mPares);

#endif