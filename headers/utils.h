#ifndef UTILS_H
#define UTILS_H

#include "../c-hashmap/map.h"
#include "../headers/dados/registro.h"

typedef struct CampoValor {
    char *campo;
    char *valor;
} CampoValor;


// ordem fixa dos campos do registro para indexação via array
#define NUM_CAMPOS_REGISTRO 8

typedef enum CampoRegistroId {
	CAMPO_COD_ESTACAO = 0,
	CAMPO_NOME_ESTACAO = 1,
	CAMPO_COD_LINHA = 2,
	CAMPO_NOME_LINHA = 3,
	CAMPO_COD_PROX_ESTACAO = 4,
	CAMPO_DIST_PROX_ESTACAO = 5,
	CAMPO_COD_LINHA_INTEGRA = 6,
	CAMPO_COD_EST_INTEGRA = 7
} CampoRegistroId;


/**
 * @brief libera a memória alocada dinamicamente de uma chave string de um hashmap
 * 
 * @param key chave string do hashmap
 * @param ksize tamanho da string chave do hashmap
 * @param value valor associado a chave
 * @param usr hashmap
 * @return int sinal indica o sucesso da operação
 */
int freeMapKeys(const void* key, size_t ksize, uintptr_t value, void* usr);


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
int confereCriteriosBusca(FILE *fileDados, Registro *reg, CampoValor *porCampo[8]);

/**
 * @brief considera como nulo um valor que seja NULL, ou uma string vazia, ou "void", ou "empty"
 * 
 * @param valor string a ser verificada
 * @return true o valor é considerado nulo
 * @return false o valor não é considerado nulo
 */
bool valorEhNulo(const char *valor);

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

/**
 * @brief encontra o índice de um campo em um array de pares campo-valor, ou -1 se não encontrado
 * 
 * @param pares array de pares a ser buscado
 * @param mPares tamanho do array de pares
 * @param campo nome do campo a ser encontrado
 * @return int índice do campo encontrado no array de pares, ou -1 se não encontrado
 */
int encontrarIndexCampo(CampoValor *pares, int mPares, const char *campo);

/**
 * @brief indexa um array de pares campo-valor por campo, preenchendo um array de ponteiros para os pares encontrados
 * 
 * @param pares array de pares a serem indexados
 * @param mPares tamanho do array de pares
 * @param out array de ponteiros para os pares encontrados, indexado por campo (posição fixa definida por CampoRegistroId), posições sem filtro ficam como NULL
 * @return int número de campos reconhecidos encontrados e indexados no array de saída
 */
int popularParesPorCampo(CampoValor *pares, int mPares, CampoValor *out[NUM_CAMPOS_REGISTRO]);

/**
 * @brief aplica um par campo-valor em um Registro (converte int, trata NULO e aloca strings quando necessário)
 * 
 * @param reg registro a ter o par aplicado
 * @param par par a ser aplicado
 * @return true par aplicado com sucesso ou campo do par desconhecido (neste caso, o par é simplesmente ignorado)
 * @return false falha de alocação ao aplicar o par
 */
bool aplicarParEmRegistro(Registro *reg, CampoValor *par);

/**
 * @brief aplica um array de pares campo-valor em um Registro (converte int, trata NULO e aloca strings quando necessário)
 * 
 * @param reg registro a ter os pares aplicados
 * @param pares array de pares a serem aplicados
 * @param mPares tamanho do array de pares
 * @return true pares aplicados com sucesso
 * @return false falha de alocação em algum dos pares
 */
bool aplicarParesEmRegistro(Registro *reg, CampoValor *pares, int mPares);

#endif