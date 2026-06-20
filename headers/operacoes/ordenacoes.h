#ifndef ORDENACOES_H
#define ORDENACOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

/**
 * @brief handler que contém a lógica de execução da ordenação externa entre os arquivos de dados das estações de metrô/trem
 * para o trabalho 3, foi liberado o uso do ".sort" (nesse caso, o qsort())
 */
void handleOrderBy();

/**
 * @brief carrega todos os registros válidos de um arquivo binário para a memória` e os ordena
 * função criada para ser reaproveitada no Sort-Merge Join
 * @param arquivoDados nome do arquivo binário de entrada
 * @param campoOrdenacao campo pelo qual os registros serão ordenados ("codEstacao" ou "codProxEstacao")
 * @param[out] qtdValidos ponteiro para armazenar a quantidade de registros carregados
 * @param[out] nroEstacoesOut ponteiro para resgatar o número de estações originais do cabeçalho (opcional para a funcionalidade SORT-MERGE JOIN)
 * @param[out] nroParesOut ponteiro para resgatar o número de pares originais do cabeçalho (opcional SORT-MERGE JOIN)
 * @return Registro* vetor dinâmico contendo os registros ordenados (NULL em caso de falha)
 */
Registro* ordenarNaMemoria(const char *arquivoDados, const char *campoOrdenacao, int *qtdValidos, int *nroEstacoesOut, int *nroParesOut);

/**
 * @brief comparador para qsort() que ordena por codEstacao, jogando nulos (-1) para o final
 * A ordenação é crescente, ou seja, o menor código de estação (exceto nulos) fica no início do arquivo
 * @param a ponteiro para o primeiro registro a ser comparado
 * @param b ponteiro para o segundo registro a ser comparado
 * @return valor negativo se a < b, zero se a == b, valor positivo se a > b, considerando a ordenação por codEstacao e
 * tratando nulos como maiores que qualquer
 */
int compararCodEstacao(const void *a, const void *b);

/**
 * @brief comparador para qsort() que ordena por codProxEstacao
 * @param a ponteiro para o primeiro registro a ser comparado
 * @param b ponteiro para o segundo registro a ser comparado
 * @return idem a compararCodEstacao(), mas considerando a ordenação por codProxEstacao
 */
int compararCodProxEstacao(const void *a, const void *b);

#endif