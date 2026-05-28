#include "../../headers/indice/btree_no.h"

No incializarNo(){
    No no;
    no.proximo = -1;
    no.tipoNo = NO_NAO_INICIALIZADO;
    no.nroChaves = 0;
    no.C1 = -1;
    no.C2 = -1;
    no.C3 = -1;
    no.Pr1 = -1;
    no.Pr2 = -1;
    no.Pr3 = -1;
    no.P1 = -1;
    no.P2 = -1;
    no.P3 = -1;
    no.P4 = -1;
    no.removido = '0';
    return no;
}
