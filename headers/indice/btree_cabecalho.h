#ifndef BTREE_CABECALHO_H
#define BTREE_CABECALHO_H

#include <stdbool.h>
#define TAM_BTREE_CABECALHO 17

/* deslocamentos fixos do registro de cabeçalho da árvore-B:
    status: 1 byte
    noRaiz: 4 bytes
    topo: 4 bytes
    proxRRN: 4 bytes
    nroNos: 4 bytes 
*/

/* 
    justificativa: a ordem dos campos no cabeçalho da árvore-B é diferente do arquivo de dados
    para facilitar a leitura e escrita dos campos específicos da árvore-B, como o RRN da raiz
    e o número de nós, sem precisar ler ou escrever os campos do arquivo de dados que não são
    relevantes para a estrutura da árvore. Além disso, manter o status no início do cabeçalho
    permite verificar rapidamente se a árvore-B está consistente antes de tentar acessar os outros campos.
*/

#define BTREE_OFF_STATUS 0
#define BTREE_OFF_NORAIZ 1
#define BTREE_OFF_TOPO 5
#define BTREE_OFF_PROXRRN 9
#define BTREE_OFF_NRONOS 13

typedef struct CabecalhoIndice {
    int noRaiz;
    int topo;
    int proxRRN;
    int nroNos;
    char status;
} CabecalhoIndice;

#endif