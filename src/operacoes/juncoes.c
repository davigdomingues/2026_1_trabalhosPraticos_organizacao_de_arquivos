#include "../../headers/operacoes/juncoes.h"
#include "../../headers/operacoes/ordenacoes.h"
#include "../../headers/dados/registro.h"
#include "../../headers/operacoes/buscas.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sortedMergeJoin(Registro *vetor1, int qtd1, Registro *vetor2, int qtd2, char *campo1, char *campo2) {
    int p1 = 0, p2 = 0; // p1: ponteiro para vetor1, p2: ponteiro para vetor2
    bool encontrou = false;

    // loop de mesclagem
    while (p1 < qtd1 && p2 < qtd2) { // comparação na base do campo de ordenação escolhido para cada vetor
        int val1 = (strcmp(campo1, "codEstacao") == 0) ? vetor1[p1].codEstacao : vetor1[p1].codProxEstacao;
        int val2 = (strcmp(campo2, "codEstacao") == 0) ? vetor2[p2].codEstacao : vetor2[p2].codProxEstacao;

        // valores nulos não se cruzam e devem ir para o final do arquivo
        if (val1 == -1) { 
            p1++; 
            continue; 
        }

        if (val2 == -1) { 
            p2++; 
            continue; 
        }

        if (val1 < val2) p1++; // ponteiro do vetor 1 avança, buscando um valor igual ou maior ao do vetor 2

        else if (val1 > val2) p2++; // mesma lógica, mas para o vetor 2

        else {
            encontrou = true;
            int p2_temp = p2;
            
            // múltiplos "matches" sem perder a referência original a p2 (relação N:M)
            while (p2_temp < qtd2) {
                int val2_temp = (strcmp(campo2, "codEstacao") == 0) ? vetor2[p2_temp].codEstacao : vetor2[p2_temp].codProxEstacao;
                if (val2_temp != val1) break;

                // formatação nos valores codEstacao, nomeEstacao, nomeLinha, codProxEstacao e nomeEstacao do segundo arquivo
                printf("%d ", vetor1[p1].codEstacao);
                printf("%s ", vetor1[p1].nomeEstacao);
                
                if (vetor1[p1].tamNomeLinha > 0) printf("%s ", vetor1[p1].nomeLinha);
                else printf("NULO ");
                
                if (vetor1[p1].codProxEstacao != -1) printf("%d ", vetor1[p1].codProxEstacao);
                else printf("NULO ");
                
                if (vetor2[p2_temp].tamNomeEstacao > 0) printf("%s\n", vetor2[p2_temp].nomeEstacao);
                else printf("NULO\n");
                
                p2_temp++;
            }
            p1++; // avança apenas p1, permitindo que o próximo p1 verifique a mesma base do p2, caso seja necessário
        }
    }

    if (!encontrou) {
        printf("Registro inexistente.\n");
    }
}

void printRegsJuncao(Registro *reg1, Registro *reg2){
    //printa os campos do registro do primeiro arquivo
    printf("%d ", reg1->codEstacao);
    printf("%s ", reg1->nomeEstacao);
    if(reg1->tamNomeLinha != 0) printf("%s ", reg1->nomeLinha);
    else printf("%s ", "NULO");
    printf("%d ", reg1->codProxEstacao);

    //printa o campo do registro 2 obtido pela junção
    if(reg2->tamNomeEstacao != 0) printf("%s ", reg2->nomeEstacao);
    else printf("%s ", "NULO");
    printf("\n");
}

void join(FILE *fileDados1, FILE *fileDados2){
    char inconsistente = '0';
    char statusDados1;
    char statusDados2;
    fread(&statusDados1, sizeof(char), 1, fileDados1);
    fread(&statusDados2, sizeof(char), 1, fileDados2);
    if(statusDados1 == inconsistente || statusDados2 == inconsistente){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    //pula o cabeçalho dos dois arquivos
    fseek(fileDados1, TAM_CABECALHO, SEEK_SET);
    fseek(fileDados2, TAM_CABECALHO, SEEK_SET);

    //estruturas em memória que vão receber os campos
    //dos registros de seus respectivos arquivos
    Registro *reg1 = (Registro*) malloc(sizeof(Registro));
    Registro *reg2 = (Registro*) malloc(sizeof(Registro));

    //indicadores de status de remoção dos registros
    char removido1;
    char removido2;

    //contador do rrn do registro atual do segundo arquivo
    //é utilizado para pular para o próx registro quando
    //o atual não satisfizer a condição de junção
    int rrn2 = -1;
    bool encontrou = false;
    while(fread(&removido1, sizeof(char), 1, fileDados1)){
        if(removido1 == '1') {
            fseek(fileDados1, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

        fseek(fileDados1, DADOS_OFF_PROXRRN - 1, SEEK_CUR); //pula os 4 bytes de proxRRN
        //lê registro do primeiro arquivo
        lerReg(fileDados1, reg1);

        while(fread(&removido2, sizeof(char), 1, fileDados2)){
            if(removido2 == '1') {
                fseek(fileDados2, TAM_REG-1, SEEK_CUR);
                continue;
            }
            rrn2++;

            //lê os campos até o codProxEstacao do registro do segundo arquivo
            fseek(fileDados2, DADOS_OFF_PROXRRN - 1, SEEK_CUR); //pula os 4 bytes de proxRRN
            fread(&reg2->codEstacao, sizeof(int), 1, fileDados2);

            //se não satisfez a condição de junção, pula
            //para o próximo registro
            if(reg1->codProxEstacao != reg2->codEstacao){
                int inicioProxReg = TAM_CABECALHO + (rrn2+1) * TAM_REG;
                fseek(fileDados2, inicioProxReg, SEEK_SET);
                continue;
            }

            encontrou = true;
            //lê o restante dos campos do registro do segundo arquivo
            fread(&reg2->codLinha, sizeof(int), 1, fileDados2);
            fread(&reg2->codProxEstacao, sizeof(int), 1, fileDados2);
            fread(&reg2->distProxEstacao, sizeof(int), 1, fileDados2);
            fread(&reg2->codLinhaIntegra, sizeof(int), 1, fileDados2);
            fread(&reg2->codEstIntegra, sizeof(int), 1, fileDados2);
            lerNomeEstacao(fileDados2, reg2);
            lerNomeLinha(fileDados2, reg2);

            //printa o resultado
            printRegsJuncao(reg1, reg2);

            //pula os $, se houver
            int tamRestante = TAM_LIVRE_REG(reg2->tamNomeEstacao, reg2->tamNomeLinha);
            if(tamRestante != 0) fseek(fileDados2, tamRestante, SEEK_CUR);
            if (reg2->tamNomeEstacao > 0) free(reg2->nomeEstacao);
            if (reg2->tamNomeLinha > 0) free(reg2->nomeLinha);
        }

        fseek(fileDados2, TAM_CABECALHO, SEEK_SET); //volta o ponteiro para o início do segundo arquivo
        rrn2 = -1; //reseta o contador de rrns do segundo arquivo

        //pula os $, se houver
        int tamRestante = TAM_LIVRE_REG(reg1->tamNomeEstacao, reg1->tamNomeLinha);
        if(tamRestante != 0) fseek(fileDados1, tamRestante, SEEK_CUR);
        if (reg1->tamNomeEstacao > 0) free(reg1->nomeEstacao);
        if (reg1->tamNomeLinha > 0) free(reg1->nomeLinha);
    }

    if(!encontrou) printf("Registro inexistente.\n");

    free(reg1);
    free(reg2);
}

void joinIndexado(FILE *fileDados1, FILE *fileDados2, FILE *fileIndice2){
    char inconsistente = '0';
    char statusDados1;
    char statusDados2;
    char statusIndice2;
    fread(&statusDados1, sizeof(char), 1, fileDados1);
    fread(&statusDados2, sizeof(char), 1, fileDados2);
    fread(&statusIndice2, sizeof(char), 1, fileIndice2);
    if(statusDados1 == inconsistente || statusDados2 == inconsistente || statusIndice2 == inconsistente){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    //pula o cabeçalho dos dois arquivos
    fseek(fileDados1, TAM_CABECALHO, SEEK_SET);
    fseek(fileDados2, TAM_CABECALHO, SEEK_SET);

    //estruturas em memória que vão receber os campos
    //dos registros de seus respectivos arquivos
    Registro *reg1 = (Registro*) malloc(sizeof(Registro));
    Registro *reg2 = (Registro*) malloc(sizeof(Registro));

    //indicadores de status de remoção dos registros
    char removido1;
    char removido2;

    //contador do rrn do registro atual do segundo arquivo
    //é utilizado para pular para o próx registro quando
    //o atual não satisfizer a condição de junção
    bool encontrou = false;
    while(fread(&removido1, sizeof(char), 1, fileDados1)){
        if(removido1 == '1') {
            fseek(fileDados1, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

        fseek(fileDados1, DADOS_OFF_PROXRRN - 1, SEEK_CUR); //pula os 4 bytes de proxRRN
        //lê registro do primeiro arquivo
        lerReg(fileDados1, reg1);

        // indexa os pares por campo (posição fixa), posições sem filtro ficam como NULL
        CampoValor *porCampo[NUM_CAMPOS_REGISTRO] = {0};
        char codProxEstacaoStr[10];
        sprintf(codProxEstacaoStr, "%d", reg1->codProxEstacao);
        CampoValor condicaoJoin = {.campo = "codEstacao", .valor = codProxEstacaoStr};
        porCampo[CAMPO_COD_ESTACAO] = &condicaoJoin;

        int byteoffsetReg = selectWhereIndexado(fileDados2, fileIndice2, porCampo, 1, false, &reg2);
        if(byteoffsetReg > -1){
            encontrou = true;
            printRegsJuncao(reg1, reg2);

            if (reg2->tamNomeEstacao > 0) free(reg2->nomeEstacao);
            if (reg2->tamNomeLinha > 0) free(reg2->nomeLinha);
            free(reg2);
        }

        fseek(fileDados2, TAM_CABECALHO, SEEK_SET); //volta o ponteiro para o início do segundo arquivo

        //pula os $, se houver
        int tamRestante = TAM_LIVRE_REG(reg1->tamNomeEstacao, reg1->tamNomeLinha);
        if(tamRestante != 0) fseek(fileDados1, tamRestante, SEEK_CUR);
        if (reg1->tamNomeEstacao > 0) free(reg1->nomeEstacao);
        if (reg1->tamNomeLinha > 0) free(reg1->nomeLinha);
    }
    free(reg1);

    if(!encontrou) printf("Registro inexistente.\n");
}

void handleJoin(){
    char *arquivo1 = NULL;
    char *arquivo2 = NULL;
    FILE *file1 = NULL;
    FILE *file2 = NULL;

    arquivo1 = lerNomeArquivo();
    if (!arquivo1){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    char codProxEstStr[32];
    scanf("%s", codProxEstStr);

    arquivo2 = lerNomeArquivo();
    if (!arquivo2){
        printf("Falha no processamento do arquivo.\n");
        free(arquivo1);
        return;
    } 

    char codEstacaoStr[32];
    scanf("%s", codEstacaoStr);

    if(strcmp(codProxEstStr, "codProxEstacao") != 0 || strcmp(codEstacaoStr, "codEstacao") != 0){
        printf("Falha no processamento do arquivo.\n");
        free(arquivo1);
        free(arquivo2);
        return;
    }

    file1 = fopen(arquivo1, "rb");
    if (!file1) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivo1);
        free(arquivo2);
        return;
    }
    
    file2 = fopen(arquivo2, "rb");
    if (!file2) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivo1);
        free(arquivo2);
        return;
    }

    join(file1, file2);

    free(arquivo1);
    free(arquivo2);

    fclose(file1);
    fclose(file2);
}

void handleIndexedJoin(){
    char *arquivoDados1 = NULL;
    char *arquivoDados2 = NULL;
    char *arquivoIndice2 = NULL;
    FILE *fileDados1 = NULL;
    FILE *fileDados2 = NULL;
    FILE *fileIndice2 = NULL;

    arquivoDados1 = lerNomeArquivo();
    if (!arquivoDados1){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    char codProxEstStr[32];
    scanf("%s", codProxEstStr);

    arquivoDados2 = lerNomeArquivo();
    if (!arquivoDados2){
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados1);
        return;
    } 

    char codEstacaoStr[32];
    scanf("%s", codEstacaoStr);

    if(strcmp(codProxEstStr, "codProxEstacao") != 0 || strcmp(codEstacaoStr, "codEstacao") != 0){
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados1);
        free(arquivoDados2);
        return;
    }


    arquivoIndice2 = lerNomeArquivo();
    if(!arquivoIndice2){
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados1);
        free(arquivoDados2);
        return;
    }

    fileDados1 = fopen(arquivoDados1, "rb");
    if (!fileDados1) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados1);
        free(arquivoDados2);
        return;
    }
    
    fileDados2 = fopen(arquivoDados2, "rb");
    if (!fileDados2) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados1);
        free(arquivoDados2);
        fclose(fileDados1);
        return;
    }

    fileIndice2 = fopen(arquivoIndice2, "rb");
    if (!fileIndice2) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados1);
        free(arquivoDados2);
        fclose(fileDados1);
        fclose(fileDados2);
        return;
    }

    joinIndexado(fileDados1, fileDados2, fileIndice2);

    free(arquivoDados1);
    free(arquivoDados2);
    free(arquivoIndice2);

    fclose(fileDados1);
    fclose(fileDados2);
    fclose(fileIndice2);
}

void handleSortMergeJoin() {
    char *arquivoDados1 = lerNomeArquivo();
    char *campo1 = lerNomeArquivo();
    char *arquivoDados2 = lerNomeArquivo();
    char *campo2 = lerNomeArquivo();

    if (!arquivoDados1 || !arquivoDados2 || !campo1 || !campo2) {
        printf("Falha no processamento do arquivo.\n");
        if (arquivoDados1) free(arquivoDados1);
        if (arquivoDados2) free(arquivoDados2);
        if (campo1) free(campo1);
        if (campo2) free(campo2);
        return;
    }

    int qtd1 = 0, qtd2 = 0;

    Registro *vetor1 = ordenarNaMemoria(arquivoDados1, campo1, &qtd1, NULL, NULL); // campo 1 é codProxEstacao
    Registro *vetor2 = ordenarNaMemoria(arquivoDados2, campo2, &qtd2, NULL, NULL); // campo 2 é codEstacao

    // limpeza de alocação, em caso de falha de alocação em um dos vetores
    if (!vetor1 || !vetor2) {
        for (int i = 0; i < qtd1; i++) {
            if (vetor1[i].tamNomeEstacao > 0) free(vetor1[i].nomeEstacao);
            if (vetor1[i].tamNomeLinha > 0) free(vetor1[i].nomeLinha);
        }
        free(vetor1);

        for (int i = 0; i < qtd2; i++) {
            if (vetor2[i].tamNomeEstacao > 0) free(vetor2[i].nomeEstacao);
            if (vetor2[i].tamNomeLinha > 0) free(vetor2[i].nomeLinha);
        }
        free(vetor2);

        free(arquivoDados1); free(arquivoDados2); free(campo1); free(campo2);
        return;
    }

    sortedMergeJoin(vetor1, qtd1, vetor2, qtd2, campo1, campo2);

    // liberação de memória
    for (int i = 0; i < qtd1; i++) {
        if (vetor1[i].tamNomeEstacao > 0) free(vetor1[i].nomeEstacao);
        if (vetor1[i].tamNomeLinha > 0) free(vetor1[i].nomeLinha);
    }
    free(vetor1);

    for (int i = 0; i < qtd2; i++) {
        if (vetor2[i].tamNomeEstacao > 0) free(vetor2[i].nomeEstacao);
        if (vetor2[i].tamNomeLinha > 0) free(vetor2[i].nomeLinha);
    }
    free(vetor2);

    free(arquivoDados1);
    free(arquivoDados2);
    free(campo1);
    free(campo2);

    return;
}