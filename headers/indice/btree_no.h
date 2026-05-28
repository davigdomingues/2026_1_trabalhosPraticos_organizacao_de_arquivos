#ifndef NO_H
#define NO_H

#include <stdbool.h>

#define TAM_NO 53
#define NO_FOLHA -1 
#define NO_RAIZ 0 
#define NO_INTERMEDIARIO 1 
#define NO_NAO_INICIALIZADO 10

typedef struct No {
    int proximo;
    int tipoNo;
    int nroChaves;
    int C1;
    int C2;
    int C3;
    int Pr1;
    int Pr2;
    int Pr3;
    int P1;
    int P2;
    int P3;
    int P4;
    char removido;
} No;

No incializarNo();

#endif
