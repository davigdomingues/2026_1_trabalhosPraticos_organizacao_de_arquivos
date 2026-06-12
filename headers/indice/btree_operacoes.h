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
#define MIN_CHAVES 1 // Em uma Árvore de ordem 4, mínimo é ceil(4/2) - 1 = 1
#define SUCESSO 1
#define CHAVE_NAO_ENCONTRADA 0
#define UNDERFLOW_PENDENTE 2

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros, bool print);
bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados);
int insertIndice(FILE *fileIndice, int chave, int ponteiroDados);
int insertIndiceRec(FILE *fileIndice, int chave, int ponteiroDados, int rrnNoAtual, ElementoIndice *elemASerPromovido);
No *split(FILE *fileIndice, No *no, ElementoIndice overflowElem, ElementoIndice *elemASerPromovido);

/**
 * @brief cria a árvore-B de índice a partir do arquivo binário de dados
 * 
 * @param arquivoDados caminho do arquivo de dados de entrada
 * @param arquivoIndice caminho do arquivo de índice de saída
 * @return true índice criado com sucesso
 * @return false falha no processamento
 */
bool criarIndiceArvoreB(char *arquivoDados, char *arquivoIndice);

/**
 * @brief Insere uma chave e seu ponteiro de dados correspondente na Árvore-B, promovendo chaves e dividindo nós conforme necessário para manter as propriedades da árvore
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param chave chave a ser inserida
 * @param ponteiroDados ponteiro para o registro no arquivo de dados
 * @return int código de sucesso ou erro da operação de inserção
 */
bool removerChaveIndice(FILE *fileIndice, int chave);

/**
 * @brief Função recursiva que percorre a Árvore-B para localizar e remover a chave especificada, realizando operações de empréstimo ou merge conforme necessário para manter as propriedades da árvore
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param rrnAtual RRN do nó atual sendo processado
 * @param chave chave a ser removida
 * @return int código indicando o resultado da remoção (sucesso, chave não encontrada, underflow pendente)
 */
int removerRecursivo(FILE *fileIndice, int rrnAtual, int chave);

/**
 * @brief Trata o underflow em um nó da Árvore-B após uma remoção, realizando empréstimo ou merge conforme necessário para restaurar as propriedades da árvore
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param pai nó pai do nó em underflow
 * @param rrnPai RRN do nó pai
 * @param indicePonteiroFilho índice do ponteiro no nó pai que aponta para o nó em underflow
 */
void tratarUnderflow(FILE *fileIndice, No *pai, int rrnPai, int indicePonteiroFilho);

/**
 * @brief Realiza o merge de dois nós da Árvore-B após uma remoção, combinando-os em um único nó
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param esq nó à esquerda
 * @param dir nó à direita
 * @param pai nó pai dos nós a serem mesclados
 * @param rrnEsq RRN do nó à esquerda
 * @param rrnDir RRN do nó à direita
 * @param rrnPai RRN do nó pai
 * @param indiceChavePai índice da chave no nó pai que separa os nós a serem mesclados
 */
void fazerMerge(FILE *fileIndice, No *esq, No *dir, No *pai, int rrnEsq, int rrnDir, int rrnPai, int indiceChavePai);

/**
 * @brief Remove registros do arquivo de dados e suas chaves da Árvore-B
 * dependendo se a chave primária foi fornecida na query (O(log n)) ou não (O(n)).
 */
bool deleteWhereIndexado(char *arquivoEntrada, char *arquivoIndice, CampoValor *pares, int mPares);

#endif