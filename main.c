#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operacoes.h"
#include "cabecalho.h"

int main(){
    FILE *file = fopen("arquivo", "rb+");
    inicializarCabecalho(file);

    FILE *csv = fopen("estacoes.csv", "r");
    char *linha = (char*) malloc(105 * sizeof(char));
    fgets(linha, 105, csv); //ignora linha de nomes das colunas
    while(fgets(linha, 105, csv) != NULL){
        char *linhaPtr = linha;
        //campos que garantidamente não são nulos
        int codEstacao = atoi(strsep(&linhaPtr, ","));
        char *nomeEstacao = strsep(&linhaPtr, ",");

        //campos possívelmente nulos
        char *codLinha = strsep(&linhaPtr, ",");
        char *nomeLinha = strsep(&linhaPtr, ",");
        char *codProxEst = strsep(&linhaPtr, ",");
        char *distanciaProxEst = strsep(&linhaPtr, ",");
        char *codLinhaInteg = strsep(&linhaPtr, ",");
        char *codEstacaoInteg = strsep(&linhaPtr, "\n\r"); //por ser o último da linha seus delimitadores são diferentes

        //trata casos nulos e insere no arquivo
        insert(file, codEstacao, nomeEstacao, codLinha, nomeLinha, codProxEst, distanciaProxEst, codLinhaInteg, codEstacaoInteg);
       //printf("CodEstacao: %d, NomeEstacao: %s, Codlinha: %d, NomeLinha: %s, CodProxEst: %d, DistanciaProxEst: %d, CodLinhaInteg: %d, CodEstacaoInteg: %d\n", codEstacao, nomeEstacao, codLinha, nomeLinha, codProxEst, distanciaProxEst, codLinhaInteg, codEstacaoInteg);
    }
    fclose(file);
    fclose(csv);
    return 0;
}