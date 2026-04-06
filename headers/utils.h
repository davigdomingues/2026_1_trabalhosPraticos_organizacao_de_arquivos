#ifndef UTILS_H
#define UTILS_H

#include "../c-hashmap/map.h"
#include "../headers/registro.h"
#include "../headers/operacoes.h"

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

bool valorEhNulo(const char *valor);
bool registroMatchParBusca(const Registro *reg, const char *nomeEstacao, const char *nomeLinha, const CampoValor *par);
int encontrarIndexCampo(CampoValor *pares, int mPares, const char *campo);

// preenche um array (tamanho NUM_CAMPOS_REGISTRO) com ponteiros para os pares encontrados
// posições sem filtro ficam como NULL e retorna quantos campos reconhecidos foram preenchidos
int popularParesPorCampo(CampoValor *pares, int mPares, CampoValor *out[NUM_CAMPOS_REGISTRO]);

// aplica um par campo-valor em um Registro (converte int, trata NULO e aloca strings quando necessário)
// retorna false apenas em falha de alocação
bool aplicarParEmRegistro(Registro *reg, CampoValor *par);

// aplicação de vários pares em sequência e retorna false apenas em falha de alocação
bool aplicarParesEmRegistro(Registro *reg, CampoValor *pares, int mPares);

#endif