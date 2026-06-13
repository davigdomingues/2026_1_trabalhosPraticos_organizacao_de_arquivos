#ifndef BUSCAS_H
#define BUSCAS_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

/**
 * @brief Dado um arquivo binário, imprime na tela todos os registros não logicamente removidos.
 * 
 * @param arquivoEntrada caminho para o arquivo binário de entrada
 */
void selectAll(FILE *file);

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

int selectAllWhere(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares);

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros, bool print);
bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados);
bool encontrarChave(No *no, int chave, int *subArvore, int *ponteiroDados);
/**
 * @brief compara o valor especificado em uma busca com o valor (int) de um registro.
 * 
 * @param index indica, por meio do sinal, se a busca especifica um valor para esse campo
 * @param valorQuery valor em string especificado na busca
 * @param valorReg valor do registro
 * @return true os valores são iguais
 * @return false os valores não são iguais ou esse campo não tem um valor especificado na busca
 */
bool verificarMatchInt(int index, char *valorQuery, int valorReg);

/**
 * @brief compara o valor especificado em uma busca com o valor (string) de um registro.
 * 
 * @param index indica, por meio do sinal, se a busca especifica um valor para esse campo
 * @param valorQuery valor em string especificado na busca
 * @param valorReg valor do registro
 * @return true os valores são iguais
 * @return false os valores não são iguais ou esse campo não tem um valor especificado na busca
 */
bool verificarMatchStr(int index, char *valorQuery, char *valorReg);

/**
 * @brief confere os critérios de busca de um registro a partir de um vetor de pares campo-valor.
 * 
 * @param fileDados arquivo binário de dados já posicionado no início do registro
 * @param reg registro a ser preenchido com os campos lidos
 * @param porCampo array de ponteiros para pares campo-valor, indexado por CampoRegistroId
 * @return int quantidade de critérios atendidos, ou -1 em falha de processamento
 */
int confereCriteriosBusca(FILE *fileDados, Registro *reg, CampoValor *porCampo[8], bool *codEstacaoMatch);

/**
 * @brief verifica se um registro corresponde a um par campo-valor de busca, considerando os campos nomeEstacao e nomeLinha para comparação de strings
 * 
 * @param reg registro a ser verificado
 * @param nomeEstacao nome da estação do registro, passado como parâmetro para comparação de strings
 * @param nomeLinha nome da linha do registro, passado como parâmetro para comparação de strings
 * @param par par campo-valor a ser comparado com o registro
 * @return true o registro corresponde ao par campo-valor de busca
 * @return false o registro não corresponde ao par campo-valor de busca
 */
bool registroMatchParBusca(const Registro *reg, const char *nomeEstacao, const char *nomeLinha, const CampoValor *par);


#endif