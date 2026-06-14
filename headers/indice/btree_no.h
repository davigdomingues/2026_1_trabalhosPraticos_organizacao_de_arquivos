#ifndef NO_H
#define NO_H

#include <stdbool.h>
#include <stdio.h>

#define TAM_NO 53 // 4 bytes para proximo + 4 para tipoNo + 4 para nroChaves + (3 chaves * 4) + (3 ponteiros de dados * 4) + (4 ponteiros de filhos * 4) + 1 para removido
#define NO_FOLHA -1 // Valor para indicar que um nó é folha
#define NO_RAIZ 0 // Valor para indicar que um nó é a raiz da árvore
#define NO_INTERMEDIARIO 1 // Valor para indicar que um nó é intermediário
#define NO_NAO_INICIALIZADO 10 // Valor para indicar que um nó não foi inicializado
#define NRO_MAX_CHAVES 3 // Ordem 4, ou seja, no máximo 3 chaves por nó
#define BTREE_NO_INICIO(rrn) (TAM_BTREE_CABECALHO + (rrn) * TAM_NO) // Macro para calcular o deslocamento inicial de um nó dado seu RRN

typedef struct No {
    int proximo;
    int tipoNo;
    int nroChaves;
    int C[3];
    int Pr[3];
    int P[4];
    char removido;
} No;

/**
 * @brief Agrupa os informações referentes a uma chave para faciltar a transferência de dados
 * 
 */
typedef struct {
    int chave;
    int ptrDados;
    int filhoDir;
} ElementoIndice;

/**
 * @brief Inicializa um nó da Árvore-B, alocando memória e definindo valores padrão para seus campos
 * @return No* ponteiro para o nó inicializado
 */
No *inicializarNo();

/**
 * @brief Lê um nó da Árvore-B a partir do arquivo de índice, dado seu RRN, e preenche a estrutura de nó fornecida
 * @param fileIndice ponteiro para o arquivo de índice
 * @param no ponteiro para a estrutura de nó que será preenchida com os dados lidos do arquivo
 */
void lerNo(FILE *fileIndice, No *no);

bool escreverNo(FILE *file, No *no);
No *criarNo(FILE *fileIndice, int *novoRRN);

/**
 * @brief Apaga um nó da Árvore-B, liberando sua memória e atualizando o contador de nós no cabeçalho do arquivo de índice
 * @param fileIndice ponteiro para o arquivo de índice
 * @param rrnNoParaApagar RRN do nó que deve ser apagado
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são removidos
 */
void apagarNo(FILE *fileIndice, int rrnNoParaApagar, int *nroNos);

#endif