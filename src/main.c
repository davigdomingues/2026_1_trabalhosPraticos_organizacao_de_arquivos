/*
    Integrantes:
    - Davi Gabriel Domingues (15447497)
    - Felipe Ferreira Colona (15636525)
*/

#include "../headers/operacoes/buscas.h"
#include "../headers/operacoes/insercoes.h"
#include "../headers/operacoes/criacoes.h"
#include "../headers/operacoes/remocoes.h"
#include "../headers/operacoes/atualizacoes.h"
#include <stdio.h>
#include <stdbool.h>

int main(){
    int op;
    scanf("%d", &op);

    switch (op) {
        case 1: // CREATE
            handleCreate();
            break;
        case 2: // SELECT ALL
            handleSelectAll();
            break;
        case 3: // SELECT ALL WHERE
            handleSelectAllWhere();
            break;
        case 4: // DELETE WHERE
            handleDeleteWhere();
            break;
        case 5: // INSERT
            handleInsert();
            break;
        case 6: // UPDATE
            handleUpdate();
            break;
        case 7: // CREATE INDEX ÁRVORE-B
            handleCreateIndex();
            break;
        case 8: //SELECT WHERE COM INDEXAÇÃO
            handleSelectWhereIndexado();
            break;
        case 9: // INSERT COM INDEXAÇÃO
            handleInsertIndexado();
            break;
        case 10: // DELETE WHERE COM INDEXAÇÃO
            handleDeleteWhereIndexado();
            break;
        case 11: // NESTED JOIN
            handleNestedJoin();
            break;
        case 12: // INDEXED JOIN
            handleIndexedJoin();
            break;
        case 13: // ORDER BY
            handleOrderBy();
            break;
        case 14: // SORT-MERGE JOIN
            handleSortMergeJoin();
            break;
        default: // operação inválida
            return -1;
    }
    return 0;
}