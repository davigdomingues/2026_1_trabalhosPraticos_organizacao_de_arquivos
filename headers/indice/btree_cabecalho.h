#ifndef BTREE_CABECALHO_H
#define BTREE_CABECALHO_H

#include <stdio.h>
#include <stdbool.h>

#define TAM_BTREE_CABECALHO 17
/* deslocamentos fixos do registro de cabeçalho da árvore-B:
    status: 1 byte
    noRaiz: 4 bytes
    topo: 4 bytes
    proxRRN: 4 bytes
    nroNos: 4 bytes 
*/

#define BTREE_OFF_STATUS 0 // 1 byte para status
#define BTREE_OFF_NORAIZ 1 // 4 bytes para noRaiz
#define BTREE_OFF_TOPO 5 // 4 bytes para topo
#define BTREE_OFF_PROXRRN 9 // 4 bytes para proxRRN
#define BTREE_OFF_NRONOS 13 // 4 bytes para nroNos

typedef struct CabecalhoIndice {
    int noRaiz;
    int topo;
    int proxRRN;
    int nroNos;
    char status;
} CabecalhoIndice;

/**
 * @brief atualiza o status do arquivo de índice da Árvore-B, escrevendo o valor especificado no cabeçalho
 * 
 * @param fileIndice ponteiro para o arquivo de índice, já aberto em modo de escrita
 * @param status valor do status a ser escrito no cabeçalho
 * @return bool indica sucesso ou falha da operação
 */
bool atualizarStatusIndice(FILE *fileIndice, char status);

/**
 * @brief escreve o cabeçalho do arquivo de índice da Árvore-B e preenche a estrutura de cabecalhoIndice fornecida
 * 
 * @param fileIndice ponteiro para o arquivo de índice, já aberto em modo de leitura
 * @param status ponteiro para armazenar o valor do status
 * @param noRaiz ponteiro para armazenar o valor do noRaiz
 * @param topo ponteiro para armazenar o valor do topo
 * @param proxRRN ponteiro para armazenar o valor do proxRRN
 * @param nroNos ponteiro para armazenar o valor do nroNos
 * @return bool indica sucesso ou falha da operação
 */
bool escreverCabecalhoIndice(FILE *fileIndice, char status, int noRaiz, int topo, int proxRRN, int nroNos);

#endif