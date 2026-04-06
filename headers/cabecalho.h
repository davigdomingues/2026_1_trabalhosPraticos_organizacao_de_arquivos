#ifndef CABECALHO_H
#define CABECALHO_H

#include <stdio.h>
#include <stdbool.h>
#define TAM_CABECALHO 17

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


void recalcularContadores(FILE *file);

#endif