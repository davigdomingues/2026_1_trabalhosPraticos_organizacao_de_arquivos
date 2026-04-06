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

/** @brief A partir de um arquivo binário, remove logicamente os registros que correspondem aos critérios de busca especificados.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 * @param pares array de campos e valores que especificam os critérios de busca
 * @param mPares tamanho do array de pares
 * @return true registros removidos com sucesso
 * @return false falha no processamento
 */
bool deleteWhere(char *arquivoEntrada, CampoValor *pares, int mPares);

/** @brief A partir de um arquivo binário, insere um novo registro com os valores especificados.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 * @param valores array de campos e valores que especificam os dados do novo registro
 * @param mValores tamanho do array de valores
 * @return true registro inserido com sucesso
 * @return false falha no processamento
 */
bool insert(char *arquivoEntrada, CampoValor *valores, int mValores);

/** @brief A partir de um arquivo binário, atualiza os registros que correspondem aos critérios de busca especificados.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 * @param arquivoSaida caminho para o arquivo binário de saída
 * @param paresBusca array de campos e valores que especificam os critérios de busca
 * @param mParesBusca tamanho do array de pares de busca
 * @param paresUpdate array de campos e valores que especificam os novos valores
 * @param mParesUpdate tamanho do array de pares de atualização
 * @return true registros atualizados com sucesso
 * @return false falha no processamento
 */
bool update(char *arquivoEntrada, char *arquivoSaida, CampoValor *paresBusca, int mParesBusca, CampoValor *paresUpdate, int mParesUpdate);

#endif