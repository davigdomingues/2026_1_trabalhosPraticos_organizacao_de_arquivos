#ifndef REMOCOES_H
#define REMOCOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"
#include "../../headers/indice/btree_no.h"

#define MIN_CHAVES 1 // Em uma Árvore de ordem 4, mínimo é ceil(4/2) - 1 = 1
#define SUCESSO 1
#define CHAVE_NAO_ENCONTRADA 0
#define UNDERFLOW_PENDENTE 2

/** @brief Dado um arquivo binário, remove logicamente os registros que correspondem aos critérios de busca especificados.
 * 
 * @param fileDados ponteiro para o arquivo binário de entrada
 * @param pares array de campos e valores que especificam os critérios de busca
 * @param mPares tamanho do array de pares
 * @return true registros removidos com sucesso
 * @return false falha no processamento
 */
bool deleteWhere(FILE *fileDados, CampoValor *pares, int mPares);

/**
 * @brief Insere uma chave e seu ponteiro de dados correspondente na Árvore-B, promovendo chaves e dividindo nós conforme necessário para manter as propriedades da árvore
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param chave chave a ser inserida
 * @param ponteiroDados ponteiro para o registro no arquivo de dados
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme novos nós são criados
 * @return int código de sucesso ou erro da operação de inserção
 */
bool removerChaveIndice(FILE *fileIndice, int chave, int *nroNos);

/**
 * @brief Função recursiva que percorre a Árvore-B para localizar e remover a chave especificada, realizando operações de empréstimo ou merge conforme necessário para manter as propriedades da árvore
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param rrnAtual RRN do nó atual sendo processado
 * @param chave chave a ser removida
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são removidos ou mesclados
 * @return int código indicando o resultado da remoção (sucesso, chave não encontrada, underflow pendente)
 */
int removerRecursivo(FILE *fileIndice, int rrnAtual, int chave, int *nroNos);

/**
 * @brief Trata o underflow em um nó da Árvore-B após uma remoção, realizando empréstimo ou merge conforme necessário para restaurar as propriedades da árvore
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param pai nó pai do nó em underflow
 * @param rrnPai RRN do nó pai
 * @param indicePonteiroFilho índice do ponteiro no nó pai que aponta para o nó em underflow
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são mesclados ou removidos
 */
void tratarUnderflow(FILE *fileIndice, No *pai, int rrnPai, int indicePonteiroFilho, int *nroNos);

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
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são mesclados ou removidos
 */
void fazerMerge(FILE *fileIndice, No *esq, No *dir, No *pai, int rrnEsq, int rrnDir, int rrnPai, int indiceChavePai, int *nroNos);

/**
 * @brief Remove registros do arquivo de dados e suas chaves da Árvore-B
 * dependendo se a chave primária foi fornecida na query (O(log n)) ou não (O(n)).
 * 
 * @param fileDados ponteiro para o arquivo de dados de entrada
 * @param fileIndice ponteiro para o arquivo de índice de saída
 * @param pares array de pares campo-valor
 * @param mPares tamanho do array de pares
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são removidos ou mesclados
 * @return bool true se a remoção foi bem-sucedida, false caso contrário
 */
bool deleteWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares, int *nroNos);

/**
 * @brief Insere uma chave e seu ponteiro de dados correspondente na Árvore-B, promovendo chaves e dividindo nós conforme necessário para manter as propriedades da árvore
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param chave chave a ser inserida
 * @param ponteiroDados ponteiro para o registro no arquivo de dados
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme novos nós são criados
 * @return int código de sucesso ou erro da operação de inserção
 */
bool removerChaveIndice(FILE *fileIndice, int chave, int *nroNos);

/**
 * @brief Função recursiva que percorre a Árvore-B para localizar e remover a chave especificada, realizando operações de empréstimo ou merge conforme necessário para manter as propriedades da árvore
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param rrnAtual RRN do nó atual sendo processado
 * @param chave chave a ser removida
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são removidos ou mesclados
 * @return int código indicando o resultado da remoção (sucesso, chave não encontrada, underflow pendente)
 */
int removerRecursivo(FILE *fileIndice, int rrnAtual, int chave, int *nroNos);

/**
 * @brief Trata o underflow em um nó da Árvore-B após uma remoção, realizando empréstimo ou merge conforme necessário para restaurar as propriedades da árvore
 * 
 * @param fileIndice arquivo de índice da Árvore-B
 * @param pai nó pai do nó em underflow
 * @param rrnPai RRN do nó pai
 * @param indicePonteiroFilho índice do ponteiro no nó pai que aponta para o nó em underflow
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são mesclados ou removidos
 */
void tratarUnderflow(FILE *fileIndice, No *pai, int rrnPai, int indicePonteiroFilho, int *nroNos);

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
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são mesclados ou removidos
 */
void fazerMerge(FILE *fileIndice, No *esq, No *dir, No *pai, int rrnEsq, int rrnDir, int rrnPai, int indiceChavePai, int *nroNos);

/**
 * @brief Remove registros do arquivo de dados e suas chaves da Árvore-B
 * dependendo se a chave primária foi fornecida na query (O(log n)) ou não (O(n)).
 * 
 * @param fileDados ponteiro para o arquivo de dados de entrada
 * @param fileIndice ponteiro para o arquivo de índice de saída
 * @param pares array de pares campo-valor
 * @param mPares tamanho do array de pares
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são removidos ou mesclados
 * @return bool true se a remoção foi bem-sucedida, false caso contrário
 */
bool deleteWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares, int *nroNos);


#endif