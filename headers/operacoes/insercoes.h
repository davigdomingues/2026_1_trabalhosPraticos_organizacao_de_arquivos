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