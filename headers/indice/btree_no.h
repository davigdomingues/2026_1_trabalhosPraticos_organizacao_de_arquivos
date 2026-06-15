#ifndef NO_H
#define NO_H

#include <stdbool.h>
#include <stdio.h>

// 4 bytes para proximo + 4 para tipoNo + 4 para nroChaves + (3 chaves * 4) + (3 ponteiros de dados * 4) 
// + (4 ponteiros de filhos * 4) + 1 para removido
#define TAM_NO 53

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

typedef struct {
    int chave;
    int ptrDados;
    int filhoDir;
} ElementoIndice;

/**
 * @brief inicializa um nó da Árvore-B, alocando memória e definindo valores padrão para seus campos
 * 
 * @return No* ponteiro para o nó inicializado
 */
No *inicializarNo();

/**
 * @brief lê um nó da Árvore-B a partir do arquivo de índice, dado seu RRN, e preenche a estrutura de nó fornecida
 * 
 * @param fileIndice ponteiro para o arquivo de índice
 * @param no ponteiro para a estrutura de nó que será preenchida com os dados lidos do arquivo
 */
void lerNo(FILE *fileIndice, No *no);

/**
 * @brief escreve um nó da Árvore-B no arquivo de índice, na posição correspondente ao seu RRN
 * 
 * @param file ponteiro para o arquivo de índice
 * @param no ponteiro para a estrutura de nó que será escrita no arquivo
 * @return bool indica sucesso ou falha da operação
 */
bool escreverNo(FILE *file, No *no);

/**
 * @brief cria um novo nó da Árvore-B, inicializando seus campos e atribuindo um RRN disponível
 * 
 * @param fileIndice ponteiro para o arquivo de índice
 * @param novoRRN ponteiro para armazenar o RRN do novo nó criado
 * @return No* ponteiro para o novo nó criado
 */
No *criarNo(FILE *fileIndice, int *novoRRN);

/**
 * @brief apaga um nó da Árvore-B, liberando sua memória e atualizando o contador de nós no cabeçalho do arquivo de índice
 * 
 * @param fileIndice ponteiro para o arquivo de índice
 * @param rrnNoParaApagar RRN do nó que deve ser apagado
 * @param nroNos ponteiro para o contador de nós da Árvore-B, que será atualizado conforme nós são removidos
 */
void apagarNo(FILE *fileIndice, int rrnNoParaApagar, int *nroNos);

/**
 * @brief limpa os campos de um nó da Árvore-B, definindo valores padrão para suas chaves, ponteiros e status de remoção
 */
void limparNo(No *no);

#endif