#ifndef CREATES_H
#define CREATES_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

void handleCreateIndex();

void handleCreate();

/**
 * @brief A partir de um arquivo CSV, cria um arquivo binário contendo registros de estações de metrô/trem.
 * 
 * @param csv ponteiro para o arquivo CSV de entrada
 * @param fileBin ponteiro para o arquivo BIN a ser criado
 * @return true arquivo BIN criado e populado com sucesso
 * @return false falha no processamento de algum dos arquivos
 */
bool create(FILE *csv, FILE *fileBin);

/**
 * @brief cria a árvore-B de índice a partir do arquivo binário de dados
 * 
 * @param fileDados ponteiro para o arquivo de dados de entrada
 * @param fileIndice ponteiro para o arquivo de índice de saída
 * @return true índice criado com sucesso
 * @return false falha no processamento
 */
bool criarIndiceArvoreB(FILE *fileDados, FILE *fileIndice);

#endif