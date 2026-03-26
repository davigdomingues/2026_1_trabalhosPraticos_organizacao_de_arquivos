#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TAM_REG 80

typedef struct Cabecalho {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacao;
} Cabecalho;

typedef struct Dados {
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int DistanciaProxEst;
    int codLinhaInteg;
    int codEstIntegra;
    int tamNomeEstacao;
    int tamNomeLinha;
    char *nomeEstacao;
    char *nomeLinha;
    char removido;
} Dados;

int main(){
    FILE *file = fopen("arquivo", "rb+");
    Cabecalho cabecalho = {.status = '1', .topo = -1, .proxRRN = 0, .nroEstacoes = 0, .nroParesEstacao = 0};
    fwrite(&cabecalho.status, sizeof(char), 1, file);
    fwrite(&cabecalho.topo, sizeof(int), 1, file);
    fwrite(&cabecalho.proxRRN, sizeof(int), 1, file);
    fwrite(&cabecalho.nroEstacoes, sizeof(int), 1, file);
    fwrite(&cabecalho.nroParesEstacao, sizeof(int), 1, file);

    FILE *csv = fopen("estacoes.csv", "r");
    char linha[105];
    fgets(linha, 105, csv); //ignora linha de nomes das colunas
    while(fgets(linha, 105, csv) != NULL){
        char *linhaPtr = linha;

        //campos que garantidamente não são nulos
        int codEstacao = atoi(strsep(&linhaPtr, ","));
        char *nomeEstacao = strsep(&linhaPtr, ",");
        int tamNomeEstacao = strlen(nomeEstacao);
        int removido = 0;
        int proximo = -1;

        //campos possívelmente nulos
        char *codLinhaNull = strsep(&linhaPtr, ",");
        char *nomeLinhaNull = strsep(&linhaPtr, ",");
        char *codProxEstNull = strsep(&linhaPtr, ",");
        char *distanciaProxEstNull = strsep(&linhaPtr, ",");
        char *codLinhaIntegNull = strsep(&linhaPtr, ",");
        char *codEstacaoIntegNull = strsep(&linhaPtr, "\n\r"); //por ser o último da linha seus delimitadores são diferentes

        //se o campo for nulo, põe -1 como valor
        int codLinha = *codLinhaNull ? atoi(codLinhaNull) : -1;
        int codProxEst = *codProxEstNull ? atoi(codProxEstNull) : -1;
        int distanciaProxEst = *distanciaProxEstNull ? atoi(distanciaProxEstNull) : -1;
        int codLinhaInteg = *codLinhaIntegNull ? atoi(codLinhaIntegNull) : -1;
        int codEstacaoInteg = *codEstacaoIntegNull ? atoi(codEstacaoIntegNull) : -1;
        

        //se o nomeLinha for nulo, põe NULL como valor e põe o tamanho como 0
        char *nomeLinha;
        int tamNomeLinha;
        if(*nomeLinhaNull != '\0'){
            nomeLinha = nomeLinhaNull;
            tamNomeLinha = strlen(nomeLinha);
        } else {
            nomeLinha = NULL;
            tamNomeLinha = 0; 
        }

        fwrite(&removido, sizeof(char), 1, file);
        fwrite(&proximo, sizeof(int), 1, file);
        fwrite(&codEstacao, sizeof(int), 1, file);
        fwrite(&codLinha, sizeof(int), 1, file);
        fwrite(&codProxEst, sizeof(int), 1, file);
        fwrite(&distanciaProxEst, sizeof(int), 1, file);
        fwrite(&codLinhaInteg, sizeof(int), 1, file);
        fwrite(&codEstacaoInteg, sizeof(int), 1, file);
        fwrite(&tamNomeEstacao, sizeof(int), 1, file);
        fwrite(nomeEstacao, sizeof(char), tamNomeEstacao, file);
        fwrite(&tamNomeLinha, sizeof(int), 1, file);
        fwrite(nomeLinha, sizeof(char), tamNomeLinha, file);

        //subtrai do tamanho total o tamanho dos campos fixos e o tamanho dos campos variáveis
        int tamRestante = TAM_REG - 9 * sizeof(int) - sizeof(char ) - tamNomeEstacao - tamNomeLinha;
        char filler[tamRestante];
        //preenche o restante do registro com '$'
        memset(filler, '$', tamRestante);
        fwrite(filler, sizeof(char), tamRestante, file);

       //printf("CodEstacao: %d, NomeEstacao: %s, Codlinha: %d, NomeLinha: %s, CodProxEst: %d, DistanciaProxEst: %d, CodLinhaInteg: %d, CodEstacaoInteg: %d\n", codEstacao, nomeEstacao, codLinha, nomeLinha, codProxEst, distanciaProxEst, codLinhaInteg, codEstacaoInteg);
    }
    fclose(file);
    fclose(csv);
    return 0;
}