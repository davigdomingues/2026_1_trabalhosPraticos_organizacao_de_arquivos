#ifndef ORDENACOES_H
#define ORDENACOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

/**
 * @brief handler que contém a lógica de execução da ordenação externa entre os arquivos de dados das estações de metrô/trem
 * para o trabalho 3, foi liberado o uso do ".sort" (algoritmo de ordenação padrão do C, que é o IntroSort)
 */
void handleOrderBy();

/**
 * @brief comparador para qsort() que ordena por codEstacao, jogando nulos (-1) para o final.
 * A ordenação é crescente, ou seja, o menor código de estação (exceto nulos) fica no início do arquivo.
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