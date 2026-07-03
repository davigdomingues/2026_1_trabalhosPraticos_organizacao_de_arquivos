#ifndef JUNCOES_H
#define JUNCOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

/**
 * @brief handler da funcionalidade de join que abre dois arquivos de dados, lê input de dois campos 
 * de condição de join e chama a funcionalidade
 */
void handleJoin();

/**
 * @brief handler da funcionalidade de join indexado que abre dois arquivos de dados, lê input de dois campos 
 * de condição de join e chama a funcionalidade
 */
void handleIndexedJoin();

/**
 * @brief handler que contém a lógica de execução da junção de mesclagem ordenada entre os arquivos de dados
 * reutiliza a função de ordenação externa, da funcionalidade ORDER BY
 */
void handleSortMergeJoin();

/**
 * @brief dado dois arquivos de dados, utiliza a técnica de junção de loop aninhado para printar
 * os registros que satisfazem a condição de junção
 * @param fileDados1: arquivo de dados 1, que tem como campo de junção o "codProxEstacao"
 * @param fileDados2: arquivo de dados 2, que tem como campo de junção o "codEstacao"
 */
void join(FILE *fileDados1, FILE *fileDados2);

/**
 * @brief dado dois arquivos de dados, utiliza a técnica de junção de loop único para printar
 * os registros que satisfazem a condição de junção
 * @param fileDados1: arquivo de dados 1, que tem como campo de junção o "codProxEstacao"
 * @param fileDados2: arquivo de dados 2, que tem como campo de junção o "codEstacao"
 * @param fileIndice2: arquivo de índice referente ao arquivo de dados 2
 */
void joinIndexado(FILE *fileDados1, FILE *fileDados2, FILE *fileIndice2);

/**
 * @brief de acordo com a formatação especificada, printa dois registros que satisfizeram
 * a condição de junção
 * @param reg1: registro 1, que tem como campo de junção o "codProxEstacao"
 * @param reg2: registro 2, que tem como campo de junção o "codEstacao"
 */
void printRegsJuncao(Registro *reg1, Registro *reg2);

#endif