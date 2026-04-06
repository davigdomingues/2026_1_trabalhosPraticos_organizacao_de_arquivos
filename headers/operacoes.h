#ifndef OPERACOES_H
#define OPERACOES_H

#include <stdio.h>
#include <stdbool.h>
#include "registro.h"

typedef struct CampoValor {
    char *campo;
    char *valor;
} CampoValor;


/**
 * @brief A partir de um arquivo CSV, cria um arquivo binário contendo registros de estações de metrô/trem.
 * 
 * @param arquivoEntrada caminho para o arquivo CSV de entrada
 * @param arquivoSaida caminho para o arquivo BIN a ser criado
 * @return true arquivo BIN criado e populado com sucesso
 * @return false falha no processamento de algum dos arquivos
 */
bool create(char *arquivoEntrada, char *arquivoSaida);


/**
 * @brief A partir de um arquivo binário, imprime na tela todos os registros não logicamente removidos.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 */
void selectAll(char *arquivoEntrada);


/**
 * @brief A partir de um arquivo binário, imprime e/ou retorna RRNs de registros que correspondem aos critérios de busca especificados.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 * @param pares array de campos e valores que especificam os critérios de busca
 * @param mPares tamanho do array de pares
 * @param rrns array de saída que é populado com os RRNs dos registros que correspondem aos critérios de busca
 * @param print flag que indica se os registros devem ser impressos ou não
 * @return int tamanho do array de RRNs
 */
int selectWhere(char *arquivoEntrada, CampoValor *par, int mPares, int **rrns, bool print);


bool deleteWhere(char *arquivoEntrada, CampoValor *pares, int mPares);
bool insert(char *arquivoEntrada, CampoValor *valores, int mValores);
bool update(char *arquivoEntrada, char *arquivoSaida, CampoValor *paresBusca, int mParesBusca, CampoValor *paresUpdate, int mParesUpdate);

#endif