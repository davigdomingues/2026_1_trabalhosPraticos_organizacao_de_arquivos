#include "../../headers/indice/btree_cabecalho.h"

bool atualizarStatusIndice(FILE *fileIndice, char status) {
    if (!fileIndice) return false;

    if (fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET) != 0) return false;
    if (fwrite(&status, sizeof(char), 1, fileIndice) != 1) return false;

    return true;
}

bool escreverCabecalhoIndice(FILE *fileIndice, char status, int noRaiz, int topo, int proxRRN, int nroNos) {
    if (!fileIndice) return false;

    if (fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET) != 0) return false;
    if (fwrite(&status, sizeof(char), 1, fileIndice) != 1) return false;
    if (fwrite(&noRaiz, sizeof(int), 1, fileIndice) != 1) return false;
    if (fwrite(&topo, sizeof(int), 1, fileIndice) != 1) return false;
    if (fwrite(&proxRRN, sizeof(int), 1, fileIndice) != 1) return false;
    if (fwrite(&nroNos, sizeof(int), 1, fileIndice) != 1) return false;

    return true;
}