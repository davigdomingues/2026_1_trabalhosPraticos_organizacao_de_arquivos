#ifndef CABECALHO_H
#define CABECALHO_H

#include <stdio.h>
#include <stdbool.h>

#define TAM_CABECALHO 17 // tamanho fixo (em bytes) que o cabeçalho ocupa no arquivo, calculado a partir dos tipos dos campos do cabeçalho
#define DADOS_OFF_STATUS 0 // offset do campo status no arquivo de dados
#define DADOS_OFF_TOPO 1 // offset do campo topo no arquivo de dados
#define DADOS_OFF_PROXRRN 5 // offset do campo proxRRN no arquivo de dados
#define DADOS_OFF_NROESTACOES 9 // offset do campo nroEstacoes no arquivo de dados
#define DADOS_OFF_NROPARES 13 // offset do campo nroParesEstacao no arquivo de dados

typedef struct Cabecalho {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacao;
} Cabecalho;

/**
 * @brief inicializa o cabeçalho em memória e escreve em um arquivo já aberto.
 * 
 * @param file arquivo aberto
 */
void inicializarCabecalho(FILE *file);

/**
 * @brief lê o valor do status do cabeçalho de um arquivo
 * 
 * @param nomeArquivo nome do arquivo a ser lido
 * @param statusOut ponteiro para armazenar o valor do status lido
 * @return true se a leitura foi bem-sucedida, false caso contrário
 */
bool lerStatusCabecalho(const char *nomeArquivo, char *statusOut);


/**
 * @brief atualiza o valor do status do cabeçalho do arquivo.
 * 
 * @param file arquivo aberto
 * @param status novo valor do status
 * @param seek indica se é necessário mover o ponteiro do arquivo, ou se ele já está no local correto
 */
void atualizarStatus(FILE *file, char status, bool seek);


/**
 * @brief atualiza o valor do proxRRN do cabeçalho do arquivo.
 * 
 * @param file arquivo aberto
 * @param status novo valor do proxRRN
 * @param seek indica se é necessário mover o ponteiro do arquivo, ou se ele já está no local correto
 */
void atualizarProxRRN(FILE *file, int proxRRN, bool seek);


/**
 * @brief atualiza o valor do nroEstacoes do cabeçalho do arquivo.
 * 
 * @param file arquivo aberto
 * @param status novo valor do nroEstacoes
 * @param seek indica se é necessário mover o ponteiro do arquivo, ou se ele já está no local correto
 */
void atualizarNroEstacoes(FILE *file, int nroEstacoes, bool seek);


/**
 * @brief atualiza o valor do nroParesEstacoes do cabeçalho do arquivo.
 * 
 * @param file arquivo aberto
 * @param status novo valor do nroParesEstacoes
 * @param seek indica se é necessário mover o ponteiro do arquivo, ou se ele já está no local correto
 */
void atualizarNroParesEstacoes(FILE *file, int nroParesEstacao, bool seek);

/**
 * @brief recalcula os contadores do cabeçalho do arquivo, lendo o arquivo e contando o número de estações e pares de estações,
 * utilizada após remoções, para garantir que os contadores estejam corretos sem a necessidade de realmente excluir os registros do arquivo
 * 
 * @param file arquivo aberto
 */
void recalcularContadores(FILE *file);

#endif