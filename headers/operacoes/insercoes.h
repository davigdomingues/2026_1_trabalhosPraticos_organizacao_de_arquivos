#ifndef INSERCOES_H
#define INSERCOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../dados/registro.h"
#include "../utils.h"
#include "../indice/btree_no.h"

#define PROMOCAO 1
#define SEM_PROMOCAO 0
#define ERRO_DE_INSERCAO -1

/**
 * @brief insere uma nova chave e ponteiro pro arquivo de dados na árvore B
 * 
 * @param fileIndice arquivo de índice
 * @param chave chave a ser inserida
 * @param ponteiroDados ponteiro pro arquivo de dados da chave a ser inserida
 * @param[in,out] nroNos número atual de nós na árvore B 
 * @return int indica sucesso da operação 
 */
int insertIndice(FILE *fileIndice, int chave, int ponteiroDados, int *nroNos);

/**
 * @brief insere recursivamente uma chave (e ponteiro pro arquivo de dados) na subárvore indicada
 * 
 * @param fileIndice arquivo de índice
 * @param chave chave a ser inserida
 * @param ponteiroDados ponteiro pro arquivo de dados da chave a ser inserida
 * @param rrnNoAtual RRN do nó que, na etapa atual da recursão, a chave vai tentar ser inserida
 * @param[out] promoPraCima elemento a ser promovido (caso ocorra um split)
 * @param[in,out] nroNos número atual de nós na árvore B 
 * @return int 1 indica que ocorreu uma promoção. 0 indica que não ocorreu uma promoção. -1 indica que ocorreu um erro de inserção
 */
int insertIndiceRec(FILE *fileIndice, int chave, int ponteiroDados, int rrnNoAtual, ElementoIndice *promoPraCima, int *nroNos);

/**
 * @brief distribui o mais uniforme possível 4 elementos entre dois nós e uma promoção
 * 
 * @param fileIndice arquivo de índice
 * @param no nó original 
 * @param novoNo nó que acabou de ser criado
 * @param rrnNovoNo RRN do nó que acabou de ser criado
 * @param overflowElem elemento que causou o overflow do nó original
 * @param[out] promoPraCima elemento a ser promovido
 */
void distribuirUniforme(FILE *fileIndice, No *no, No *novoNo, int rrnNovoNo, ElementoIndice overflowElem, ElementoIndice *promoPraCima);

/**
 * @brief insere um elemento em nó com espaço sobrando
 * 
 * @param no nó a recever o elemento
 * @param elem elemento a ser inserido
 * @return true elemento foi inserido
 * @return false elemento não foi inserido (não tinha espaço sobrando)
 */
bool insereOrdenado(No *no, ElementoIndice elem);

/**
 * @brief cria uma nova raiz para árvore B
 * 
 * @param fileIndice arquivo de índice
 * @param rrnRaizAtual RRN da raiz atual
 * @param promoDeBaixo elemento que vai ser promovido a raiz
 * @param[in,out] nroNos número de nós atual da árvore B
 */
void criarNovaRaiz(FILE *fileIndice, int rrnRaizAtual, ElementoIndice promoDeBaixo, int *nroNos);

/** 
 * @brief realiza o split de um nó com overflow na árvore B, criando um novo nó e indicando o elemento a ser promovido
 * 
 * @param fileIndice arquivo de índice
 * @param no nó que sofreu overflow
 * @param overflowElem elemento que causou o overflow
 * @param[out] promoPraCima elemento que vai ser promovido para o nó pai
 * @return No* ponteiro para o novo nó criado a partir do split
 */
No *split(FILE *fileIndice, No *no, ElementoIndice overflowElem, ElementoIndice *promoPraCima);

/**
 * @brief auxiliar que calcular o número de estações únicas e o número de pares estação-valor para cada estação, a partir 
 * do arquivo de dados
 * 
 * @param fileDados arquivo de dados
 * @param[out] nroEstacoes número de estações únicas encontrado
 * @param[out] nroParesEstacao número total de pares estação-valor encontrado
 * @return true cálculo realizado com sucesso
 * @return false falha no processamento
 */
bool calculaNroEstacoesUnicas(FILE *fileDados, int *nroEstacoes, int *nroParesEstacao);

/**
 * @brief handler para a operação de inserção, que lê os campos e valores do novo registro a ser inserido no arquivo de dados
 */
void handleInsert();

/**
 * @brief handler para a operação de inserção indexada, que lê os campos e valores do novo registro a ser inserido na árvore B
 */
void handleInsertIndexado();

/** 
 * @brief insere um novo registro no arquivo de dados e atualiza o arquivo de índice da árvore B.
 * 
 * @param fileDados arquivo de dados
 * @param fileIndice arquivo de índice
 * @param valores array de campos e valores que especificam os dados do novo registro
 * @param mValores tamanho do array de valores
 * @param[in,out] nroNos número atual de nós na árvore B (pode ser incrementado caso ocorra split)
 * @return true registro inserido com sucesso
 * @return false falha no processamento ou na inserção
 */
bool insert(FILE *fileDados, FILE *fileIndice, CampoValor *valores, int mValores, int *nroNos);

#endif