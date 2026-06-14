#ifndef BUSCAS_H
#define BUSCAS_H

#include <stdio.h>
#include <stdbool.h>
#include "../utils.h"
#include "../dados/registro.h"

/**
 * @brief Dado um arquivo binário, imprime na tela todos os registros não logicamente removidos.
 * * @param file ponteiro para o arquivo binário de entrada aberto
 */
void selectAll(FILE *file);

/**
 * @brief Dado um arquivo binário, imprime os registros que satisfazem os critérios de busca ou retorna o rrn do primeiro registro que os satisfizer.
 * * @param fileDados ponteiro para o arquivo de dados aberto em modo "rb"
 * @param fileIndice ponteiro para o arquivo de índice aberto em modo "rb"
 * @param par ponteiro para a estrutura de campo e valor que especifica o critério de busca
 * @param mPares tamanho do array de pares (ou número de critérios)
 * @param rrnInicial rrn que indica de qual registro a busca deve iniciar
 * @param apenasPrimeiroRes flag que indica se deve retornar apenas o primeiro resultado encontrado
 * @param seek flag que indica se deve ser feito uma chamada à fseek ou se o ponteiro já está na posição correta
 * @return int rrn do primeiro resultado encontrado ou indicador de sucesso/falha da operação
 */
int selectWhere(FILE *fileDados, FILE *fileIndice, CampoValor *par, int mPares, int rrnInicial, bool apenasPrimeiroRes, bool seek);

/**
 * @brief Dado um arquivo binário, imprime na tela todos os registros não logicamente removidos que satisfazem os critérios de busca.
 * * @param fileDados ponteiro para o arquivo binário de dados de entrada
 * @param fileIndice ponteiro para o arquivo binário de índice de entrada
 * @param pares array de campos e valores que especificam os critérios de busca
 * @param mPares tamanho do array de pares
 * @return int sinal indica sucesso ou falha da operação
 */
int selectAllWhere(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares);

/**
 * @brief compara o valor especificado em uma busca com o valor (int) de um registro.
 * * @param index indica, por meio do sinal, se a busca especifica um valor para esse campo
 * @param valorQuery valor em string especificado na busca
 * @param valorReg valor do registro
 * @return true os valores são iguais
 * @return false os valores não são iguais ou esse campo não tem um valor especificado na busca
 */
bool verificarMatchInt(int index, char *valorQuery, int valorReg);

/**
 * @brief compara o valor especificado em uma busca com o valor (string) de um registro.
 * * @param index indica, por meio do sinal, se a busca especifica um valor para esse campo
 * @param valorQuery valor em string especificado na busca
 * @param valorReg valor do registro
 * @return true os valores são iguais
 * @return false os valores não são iguais ou esse campo não tem um valor especificado na busca
 */
bool verificarMatchStr(int index, char *valorQuery, char *valorReg);

/**
 * @brief confere os critérios de busca de um registro a partir de um vetor de pares campo-valor.
 * * @param fileDados arquivo binário de dados já posicionado no início do registro
 * @param reg registro a ser preenchido com os campos lidos
 * @param porCampo array de ponteiros para pares campo-valor, indexado por CampoRegistroId
 * @param[out] codEstacaoMatch flag que indica se um dos matches foi de codEstacao
 * @return int quantidade de critérios atendidos, ou -1 em falha de processamento
 */
int confereCriteriosBusca(FILE *fileDados, Registro *reg, CampoValor *porCampo[8], bool *codEstacaoMatch);

/**
 * @brief executa uma busca indexada (caso um dos critérios de busca seja codEstacao) ou executa uma busca sequencial
 * * @param fileDados arquivo binário de dados
 * @param fileIndice arquivo binário de índice
 * @param pares array de exatamente 8 ponteiros para pares campo-valor que são os critérios de busca
 * @param numFiltros tamanho do array pares
 * @param print flag que indica se os registros que satisfazem os critérios de busca devem ser printados 
 * @return int sinal indica sucesso ou falha da operação
 */
int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros, bool print);

/**
 * @brief busca recursivamente uma chave em uma árvore B
 * * @param fileIndice arquivo binário de índice
 * @param chave chave a ser buscada
 * @param rrnNoAtual RRN do nó que, na etapa atual da recursão, a chave vai ser buscada
 * @param[out] rrnNoRes RRN do nó cuja chave foi encontrada
 * @param[out] ponteiroDados ponteiro de dados da chave encontrada
 * @return true chave encontrada
 * @return false chave não encontrada
 */
bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados);

/**
 * @brief busca uma chave em um nó de uma árvore B e, caso não seja encontrada, indica a subárvore em que a próxima busca deve ser feita
 * * @param no nó da árvore B em que a busca será feita
 * @param chave chave a ser buscada
 * @param[out] subArvore subárvore em que a próxima busca deve ser feita
 * @param[out] ponteiroDados ponteiro de dados da chave encontrada
 * @return true chave encontrada
 * @return false chave não encontrada
 */
bool encontrarChave(No *no, int chave, int *subArvore, int *ponteiroDados);

/**
 * @brief verifica se um registro corresponde a um par campo-valor de busca, considerando os campos nomeEstacao e nomeLinha para comparação de strings
 * * @param reg registro a ser verificado
 * @param nomeEstacao nome da estação do registro, passado como parâmetro para comparação de strings
 * @param nomeLinha nome da linha do registro, passado como parâmetro para comparação de strings
 * @param par par campo-valor a ser comparado com o registro
 * @return true o registro corresponde ao par campo-valor de busca
 * @return false o registro não corresponde ao par campo-valor de busca
 */
bool registroMatchParBusca(const Registro *reg, const char *nomeEstacao, const char *nomeLinha, const CampoValor *par);

#endif