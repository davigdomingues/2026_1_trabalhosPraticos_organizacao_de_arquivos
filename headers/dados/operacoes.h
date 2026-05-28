#ifndef OPERACOES_H
#define OPERACOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "registro.h"

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
 * @brief Dado um arquivo binário, imprime na tela todos os registros não logicamente removidos.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 */
void selectAll(char *arquivoEntrada);


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

int selectAllWhere(char *arquivoDados, char *arquivoIndice, CampoValor *pares, int mPares);


/** @brief Dado um arquivo binário, remove logicamente os registros que correspondem aos critérios de busca especificados.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 * @param pares array de campos e valores que especificam os critérios de busca
 * @param mPares tamanho do array de pares
 * @return true registros removidos com sucesso
 * @return false falha no processamento
 */

bool deleteWhere(char *arquivoEntrada, CampoValor *pares, int mPares);


/** @brief Dado um arquivo binário, insere um novo registro com os valores especificados.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 * @param valores array de campos e valores que especificam os dados do novo registro
 * @param mValores tamanho do array de valores
 * @return true registro inserido com sucesso
 * @return false falha no processamento
 */
bool insert(char *arquivoEntrada, CampoValor *valores, int mValores);


/** @brief Dado um arquivo binário, atualiza os registros que correspondem aos critérios de busca especificados.
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


int confereCriteriosBusca(FILE *fileDados, Registro *reg, CampoValor *porCampo[8]);

#endif