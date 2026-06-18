#ifndef JUNCOES_H
#define JUNCOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

/**
 * @brief handler que contém a lógica de execução da junção aninhada (nested join) entre os arquivos de dados das estações de metrô/trem
 * propositalmente estipulado para ter desempenho O(n^2)
 */
void handleNestedJoin();

/**
 * @brief handler que contém a lógica de execução da junção indexada entre os arquivos de dados
 * propositalmente estipulado para ter desempenho O(n*log n)
 */
void handleIndexedJoin();

/**
 * @brief handler que contém a lógica de execução da junção de mesclagem ordenada entre os arquivos de dados
 * reutiliza a função de ordenação externa, da funcionalidade ORDER BY
 */
void handleSortMergeJoin();

#endif