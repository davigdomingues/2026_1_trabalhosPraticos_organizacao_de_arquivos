#ifndef JUNCOES_H
#define JUNCOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

/**
 * @brief handler que contém a lógica de execução da junção aninhada (nested join) entre os arquivos de dados das estações de metrô/trem
 * o desempenho geral é O(n*m), onde n e m são as quantidades de registros válidos nos arquivos de dados envolvidos na junção, mas 
 * O(n²) para o pior caso
 */
void handleJoin();

/**
 * @brief handler que contém a lógica de execução da junção indexada entre os arquivos de dados
 * propositalmente estipulado para ter desempenho geral O(n*log n)
 */
void handleIndexedJoin();

/**
 * @brief handler que contém a lógica de execução da junção de mesclagem ordenada entre os arquivos de dados
 * reutiliza a função de ordenação externa, da funcionalidade ORDER BY
 */
void handleSortMergeJoin();

#endif