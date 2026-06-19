#include "../../headers/operacoes/ordenacoes.h"
#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/utils.h"
#include "../../headers/fornecidas.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compararCodEstacao(const void *a, const void *b) {
    Registro *r1 = (Registro *)a; // registro1, primeiro a ser comparado
    Registro *r2 = (Registro *)b; // registro2, segundo a ser comparado

    if (r1->codEstacao == -1 && r2->codEstacao != -1) return 1; // r1 nulo vai pra direita
    if (r1->codEstacao != -1 && r2->codEstacao == -1) return -1; // r2 nulo fica na esquerda
    
    return r1->codEstacao - r2->codEstacao;
}

int compararCodProxEstacao(const void *a, const void *b) { // mesma lógica de compararCodEstacao, mas para codProxEstacao
    Registro *r1 = (Registro *)a;
    Registro *r2 = (Registro *)b;

    if (r1->codProxEstacao == -1 && r2->codProxEstacao != -1) return 1;
    if (r1->codProxEstacao != -1 && r2->codProxEstacao == -1) return -1;
    
    return r1->codProxEstacao - r2->codProxEstacao;
}

void handleOrderBy() {
    char *arquivoDados = lerNomeArquivo();
    char *campoOrdenacao = lerNomeArquivo();
    char *arquivoSaida = lerNomeArquivo();

    if (!arquivoDados || !campoOrdenacao || !arquivoSaida) {
        printf("Falha no processamento do arquivo.\n");
        if (arquivoDados) free(arquivoDados);
        if (campoOrdenacao) free(campoOrdenacao);
        if (arquivoSaida) free(arquivoSaida);
        return;
    }

    FILE *fileDados = fopen(arquivoDados, "rb");
    if (!fileDados) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados); free(campoOrdenacao); free(arquivoSaida);
        return;
    }

    // leitura segura do cabeçalho e verificação de consistência
    char status;
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    if (fread(&status, sizeof(char), 1, fileDados) != 1 || status != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados);
        free(arquivoDados); free(campoOrdenacao); free(arquivoSaida);
        return;
    }

    // leitura dos dados do cabeçalho, que serão reutilizados no arquivo de saída
    int topo, proxRRN, nroEstacoes, nroPares;
    fread(&topo, sizeof(int), 1, fileDados);
    fread(&proxRRN, sizeof(int), 1, fileDados);
    fread(&nroEstacoes, sizeof(int), 1, fileDados);
    fread(&nroPares, sizeof(int), 1, fileDados);

    // uso de vetor dinâmico com tamanho razoável inicial
    int capacidade = 100;
    Registro *vetor = (Registro*) malloc(capacidade * sizeof(Registro));
    if (!vetor) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados);
        free(arquivoDados); free(campoOrdenacao); free(arquivoSaida);
        return;
    }

    fseek(fileDados, TAM_CABECALHO, SEEK_SET);

    int qtdValidos = 0; // contador de registros válidos lidos, que também indica a posição de inserção no vetor
    char removido;

    // extração em varredura que retira apenas registros válidos
    while (fread(&removido, sizeof(char), 1, fileDados) == 1) {
        if (removido == '1') {
            fseek(fileDados, TAM_REG - 1, SEEK_CUR);
            continue;
        }

        Registro reg = inicializarReg();
        reg.removido = '0';

        if (fread(&reg.proximo, sizeof(int), 1, fileDados) != 1) break;

        fread(&reg.codEstacao, sizeof(int), 1, fileDados);
        fread(&reg.codLinha, sizeof(int), 1, fileDados);
        fread(&reg.codProxEstacao, sizeof(int), 1, fileDados);
        fread(&reg.distProxEstacao, sizeof(int), 1, fileDados);
        fread(&reg.codLinhaIntegra, sizeof(int), 1, fileDados);
        fread(&reg.codEstIntegra, sizeof(int), 1, fileDados);

        fread(&reg.tamNomeEstacao, sizeof(int), 1, fileDados);
        if (reg.tamNomeEstacao > 0) {
            reg.nomeEstacao = (char*) malloc(reg.tamNomeEstacao + 1);
            fread(reg.nomeEstacao, sizeof(char), reg.tamNomeEstacao, fileDados);
            reg.nomeEstacao[reg.tamNomeEstacao] = '\0';
        }

        fread(&reg.tamNomeLinha, sizeof(int), 1, fileDados);
        if (reg.tamNomeLinha > 0) {
            reg.nomeLinha = (char*) malloc(reg.tamNomeLinha + 1);
            fread(reg.nomeLinha, sizeof(char), reg.tamNomeLinha, fileDados);
            reg.nomeLinha[reg.tamNomeLinha] = '\0';
        }

        // pula o lixo de memória restante do registro, se houver
        int tamRestante = TAM_LIVRE_REG(reg.tamNomeEstacao, reg.tamNomeLinha);
        if (tamRestante > 0) fseek(fileDados, tamRestante, SEEK_CUR);

        // alocação dinâmica do vetor, que dobra de tamanho quando necessário
        if (qtdValidos >= capacidade) {
            capacidade *= 2;
            Registro *temp = (Registro*) realloc(vetor, capacidade * sizeof(Registro));
            if (!temp) break;
            vetor = temp;
        }

        vetor[qtdValidos++] = reg;
    }
    fclose(fileDados);

    // uso do qsort() para ordenação, com tratamento de campo inválido que tenta processar o que já foi lido
    if (strcmp(campoOrdenacao, "codEstacao") == 0) qsort(vetor, qtdValidos, sizeof(Registro), compararCodEstacao);
    
    else if (strcmp(campoOrdenacao, "codProxEstacao") == 0 || strcmp(campoOrdenacao, "codProxEst") == 0) qsort(vetor, qtdValidos, sizeof(Registro), compararCodProxEstacao);
    
    else { // campo de ordenação inválido, necessidade de liberar memória
        printf("Falha no processamento do arquivo.\n");
        for (int i = 0; i < qtdValidos; i++) {
            if (vetor[i].tamNomeEstacao > 0) free(vetor[i].nomeEstacao);
            if (vetor[i].tamNomeLinha > 0) free(vetor[i].nomeLinha);
        }
        free(vetor);
        free(arquivoDados); free(campoOrdenacao); free(arquivoSaida);
        return;
    }

    FILE *fileSaida = fopen(arquivoSaida, "wb");
    if (!fileSaida) {
        printf("Falha no processamento do arquivo.\n");
        for (int i = 0; i < qtdValidos; i++) {
            if (vetor[i].tamNomeEstacao > 0) free(vetor[i].nomeEstacao);
            if (vetor[i].tamNomeLinha > 0) free(vetor[i].nomeLinha);
        }
        free(vetor);
        free(arquivoDados); free(campoOrdenacao); free(arquivoSaida);
        return;
    }

    // gravação inicial de inconsistência
    char novoStatus = '0';
    int novoTopo = -1;
    fwrite(&novoStatus, sizeof(char), 1, fileSaida);
    fwrite(&novoTopo, sizeof(int), 1, fileSaida);
    fwrite(&qtdValidos, sizeof(int), 1, fileSaida); 
    fwrite(&nroEstacoes, sizeof(int), 1, fileSaida); 
    fwrite(&nroPares, sizeof(int), 1, fileSaida); 

    // blocos ordenados são gravados e a memória alocada é liberada
    for (int i = 0; i < qtdValidos; i++) {
        escreverReg(fileSaida, &vetor[i]);
        if (vetor[i].tamNomeEstacao > 0) free(vetor[i].nomeEstacao);
        if (vetor[i].tamNomeLinha > 0) free(vetor[i].nomeLinha);
    }
    
    // arquivo confirmado como consistente
    novoStatus = '1';
    fseek(fileSaida, DADOS_OFF_STATUS, SEEK_SET);
    fwrite(&novoStatus, sizeof(char), 1, fileSaida);
    fclose(fileSaida);

    BinarioNaTela(arquivoSaida);

    free(vetor);
    free(arquivoDados);
    free(campoOrdenacao);
    free(arquivoSaida);
}