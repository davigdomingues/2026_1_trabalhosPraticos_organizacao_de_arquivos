#include "../../headers/indice/btree_cabecalho.h"

bool atualizarStatusIndice(FILE *fileIndice, char status) {
    if (!fileIndice) return false;

    // atualiza o status de consistência do arquivo de índice, escrevendo o novo status no byte reservado para isso no cabeçalho
    if (fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET) != 0) return false;
    if (fwrite(&status, sizeof(char), 1, fileIndice) != 1) return false;

    return true;
}

bool escreverCabecalhoIndice(FILE *fileIndice, char status, int noRaiz, int topo, int proxRRN, int nroNos) {
    if (!fileIndice) return false;

    if (fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET) != 0) return false; // garante que o ponteiro de escrita está no início do cabeçalho
    if (fwrite(&status, sizeof(char), 1, fileIndice) != 1) return false; // escreve o status de consistência
    if (fwrite(&noRaiz, sizeof(int), 1, fileIndice) != 1) return false; // escreve o RRN da raiz
    if (fwrite(&topo, sizeof(int), 1, fileIndice) != 1) return false; // escreve o RRN do topo da pilha de nós removidos
    if (fwrite(&proxRRN, sizeof(int), 1, fileIndice) != 1) return false; // escreve o próximo RRN disponível para escrita de um novo nó
    if (fwrite(&nroNos, sizeof(int), 1, fileIndice) != 1) return false; // escreve o número de nós atualmente no arquivo

    return true;
}