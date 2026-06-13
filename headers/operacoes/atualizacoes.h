#ifndef ATUALIZACOES_H
#define ATUALIZACOES_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

/** @brief Dado um arquivo binário, atualiza os registros que correspondem aos critérios de busca especificados.
 * 
 * @param fileDados ponteiro para o arquivo binário de entrada
 * @param paresBusca array de campos e valores que especificam os critérios de busca
 * @param mParesBusca tamanho do array de pares de busca
 * @param paresUpdate array de campos e valores que especificam os novos valores
 * @param mParesUpdate tamanho do array de pares de atualização
 * @return true registros atualizados com sucesso
 * @return false falha no processamento
 */
bool update(FILE *fileDados, CampoValor *paresBusca, int mParesBusca, CampoValor *paresUpdate, int mParesUpdate);

#endif