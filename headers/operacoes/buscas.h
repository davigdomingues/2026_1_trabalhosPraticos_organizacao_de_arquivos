#ifndef BUSCAS_H
#define BUSCAS_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

/**
 * @brief Dado um arquivo binário, imprime na tela todos os registros não logicamente removidos.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 */
void selectAll(FILE *file);


/**
 * @brief Dado um arquivo binário, imprime os registros que satisfazem os critérios de busca ou retorna o rrn do primeiro registro que os satisfizer.
 * 
 * @param file ponteiro para o arquivo aberto em modo "rb"
 * @param pares array de campos e valores que especificam os critérios de busca
 * @param mPares tamanho do array de pares
 * @param rrnInicial rrn que indica de qual registro a busca deve iniciar
 * @param apenasPrimeiroRes flag que indica se deve retornar apenas o primeiro resultado encontrado
 * @param seek flag que indica se deve ser feito uma chamada à fseek ou se o ponteiro já está na posição correta
 * @return int rrn do primeiro resultado encontrado ou, simplesmente, indicador de sucesso da operação (sinal do int)
 */
int selectWhere(FILE *fileDados, FILE *fileIndice, CampoValor *par, int mPares, int rrnInicial, bool apenasPrimeiroRes, bool seek);

/**
 * @brief Dado um arquivo binário, imprime na tela todos os registros não logicamente removidos que satisfazem os critérios de busca.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 * @param pares array de campos e valores que especificam os critérios de busca
 * @param mPares tamanho do array de pares
 * @return int sinal indica sucesso ou falha da operação
 */

int selectAllWhere(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares);

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros, bool print);
bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados);

#endif