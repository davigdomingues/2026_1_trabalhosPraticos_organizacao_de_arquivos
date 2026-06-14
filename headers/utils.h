#ifndef UTILS_H
#define UTILS_H

#include "../c-hashmap/map.h"
#include "../headers/dados/registro.h"
#include "../headers/indice/btree_no.h"

typedef struct CampoValor {
    char *campo;
    char *valor;
} CampoValor;


// ordem fixa dos campos do registro para indexação via array
#define NUM_CAMPOS_REGISTRO 8

#define TAM_ARQUIVO 100 // tamanho máximo para nome de arquivo
#define TAM_CAMPO 20
#define TAM_VALOR 50
#define MAX_PARES 8 // número máximo de pares


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

/** @brief lê um nome de arquivo (token sem espaços) e devolve uma string alocada
 * 
 * @return ponteiro para a string alocada, ou NULL em caso de erro
 */
char *lerNomeArquivo(void);

/** @brief libera a memória alocada para os campos e os valores de um array de CampoValor
 * 
 * @param pares array de CampoValor a ter sua memória liberada
 * @param mPares tamanho do array de pares
 */
void liberarPares(CampoValor *pares, int mPares);


/** @brief lê os pares campo-valor da entrada padrão e armazená-los em um array de CampoValor
 * 
 * @param pares array de CampoValor a ser preenchido
 * @param mPares tamanho do array de pares
 */
void lerPares(CampoValor *pares, int mPares);

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
 * @brief considera como nulo um valor que seja NULL, ou uma string vazia, ou "void", ou "empty"
 * 
 * @param valor string a ser verificada
 * @return true o valor é considerado nulo
 * @return false o valor não é considerado nulo
 */
bool valorEhNulo(const char *valor);

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

/**
 * @brief ordena um array de ElementoIndice em ordem crescente de chave usando o algoritmo de ordenação por inserção
 * 
 * @param arr array de ElementoIndice a ser ordenado
 * @param tam tamanho do array a ser ordenado
 */
void insertionSort(ElementoIndice arr[], int tam);

#endif