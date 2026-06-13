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

int insertIndice(FILE *fileIndice, int chave, int ponteiroDados, int *nroNovosNos);
int insertIndiceRec(FILE *fileIndice, int chave, int ponteiroDados, int rrnNoAtual, ElementoIndice *elemASerPromovido, int *nroNos);
void distribuirUniforme(FILE *fileIndice, No *no, No *novoNo, int rrnNovoNo, ElementoIndice overflowElem, ElementoIndice *promoPraCima);
bool encontrarChave(No *no, int chave, int *subArvore, int *ponteiroDados);
bool insereOrdenado(No *no, ElementoIndice elem);

/** @brief Dado um arquivo binário, insere um novo registro com os valores especificados.
 * 
 * @param fileIndice ponteiro para o arquivo de índice
 * @param no ponteiro para o nó onde a inserção deve ocorrer
 * @param overflowElem elemento que causou o overflow e deve ser inserido
 * @param elemASerPromovido elemento que deve ser promovido para o nó pai em caso de overflow
 * @return No* ponteiro para o novo nó criado em caso de split, ou NULL caso contrário
 */
No *split(FILE *fileIndice, No *no, ElementoIndice overflowElem, ElementoIndice *elemASerPromovido);

/** @brief Dado um arquivo binário, insere um novo registro com os valores especificados.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 * @param valores array de campos e valores que especificam os dados do novo registro
 * @param mValores tamanho do array de valores
 * @return true registro inserido com sucesso
 * @return false falha no processamento
 */
bool insert(FILE *fileDados, FILE *fileIndice, CampoValor *valores, int mValores, int *nroNos);

#endif