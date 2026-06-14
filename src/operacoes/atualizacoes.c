#include "../../headers/operacoes/atualizacoes.h"
#include "../../headers/operacoes/buscas.h"
#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/utils.h"
#include "../../headers/fornecidas.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleUpdate(){
    char *arquivoDados = NULL;
    FILE *fileDados = NULL;
    bool ok = false;

    arquivoDados = lerNomeArquivo();
    if (!arquivoDados) return;

    fileDados = fopen(arquivoDados, "r+b");
    if (!fileDados) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados);
        return;
    }

    // Validação de consistência do cabeçalho
    char statusUpdSeq;
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fread(&statusUpdSeq, sizeof(char), 1, fileDados);
    if (statusUpdSeq != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados);
        free(arquivoDados);
        return;
    }

    // Modifica status para modulação inconsistente em lote
    char statusInconsistenteUpd = '0';
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fwrite(&statusInconsistenteUpd, sizeof(char), 1, fileDados);

    int nAtualizacoes = 0; // número de operações de atualização a serem realizadas
    scanf("%d", &nAtualizacoes);

    // busca e atualização (pares campo-valor)
    CampoValor *paresBusca = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
    CampoValor *paresUpdate = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);

    // loop das operações de atualização
    ok = true;

    for (int i = 0; i < nAtualizacoes; i++) {
        int mParesBusca = 0;
        scanf("%d", &mParesBusca);
        lerPares(paresBusca, mParesBusca);

        int mParesUpdate = 0; // número de pares para a parte de atualização da operação de atualização atual
        scanf("%d", &mParesUpdate);
        lerPares(paresUpdate, mParesUpdate);

        // se houver falha em alguma das atualizações, ok = false e as próximas atualizações não são tentadas
        if (ok && !update(fileDados, paresBusca, mParesBusca, paresUpdate, mParesUpdate)) {
            ok = false;
        }

        // liberação dos pares da operação de atualização atual antes de ler os próximos
        liberarPares(paresBusca, mParesBusca);
        liberarPares(paresUpdate, mParesUpdate);

        if (!ok) break; // encerra antes de completar as N atualizações
    }

    free(paresBusca);
    free(paresUpdate);

    // Atualiza contadores uma única vez ao término e redefine a consistência do cabeçalho
    if (ok) {
        recalcularContadores(fileDados);
        
        char statusConsistente = '1';
        fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
        fwrite(&statusConsistente, sizeof(char), 1, fileDados);
    }

    fclose(fileDados);
    BinarioNaTela(arquivoDados);
    free(arquivoDados);
    return;
}


bool update(FILE *fileDados, CampoValor *paresBusca, int mParesBusca, CampoValor *paresUpdate, int mParesUpdate) {
    int rrn = -1;
    bool ok = true;

    while(true){
        // busca o próximo registro que satisfaz os critérios para atualização
        // +1 para não ficar retornando sempre o mesmo rrn
        // seek = false, porque o ponteiro já estará no início do próximo rrn ao final do loop
        rrn = selectWhere(fileDados, NULL, paresBusca, mParesBusca, rrn+1, true, false);
        if (rrn < 0) break;

        // cálculo do byte offset exato do registro
        long inicioRegistro = (long)TAM_CABECALHO + (long)rrn * (long)TAM_REG;
        
        if (fseek(fileDados, inicioRegistro, SEEK_SET) != 0) { 
            ok = false; 
            break; 
        }

        // lê a flag de removido para garantir que o registro não foi marcado como removido entre a busca e a atualização
        char removido;
        if (fread(&removido, sizeof(char), 1, fileDados) != 1) { 
            ok = false; 
            break; 
        }

        Registro reg;
        reg.removido = '0';

        // leitura dos campos do registro original
        if (fread(&reg.proximo, sizeof(int), 1, fileDados) != 1) { ok = false; break; }
        if (fread(&reg.codEstacao, sizeof(int), 1, fileDados) != 1) { ok = false; break; }
        if (fread(&reg.codLinha, sizeof(int), 1, fileDados) != 1) { ok = false; break; }
        if (fread(&reg.codProxEstacao, sizeof(int), 1, fileDados) != 1) { ok = false; break; }
        if (fread(&reg.distProxEstacao, sizeof(int), 1, fileDados) != 1) { ok = false; break; }
        if (fread(&reg.codLinhaIntegra, sizeof(int), 1, fileDados) != 1) { ok = false; break; }
        if (fread(&reg.codEstIntegra, sizeof(int), 1, fileDados) != 1) { ok = false; break; }

        // leitura dos campos de string, alocando dinamicamente e tratando os casos de campos nulos
        char *nomeEstacao = "";
        if (fread(&reg.tamNomeEstacao, sizeof(int), 1, fileDados) != 1) { ok = false; break; }
        // se o campo não for nulo, aloca memória para a string, lê do arquivo e garante a terminação nula
        if (reg.tamNomeEstacao > 0) {
            nomeEstacao = (char*) malloc((size_t)reg.tamNomeEstacao + 1);
            if (!nomeEstacao) { ok = false; break; }
            if (fread(nomeEstacao, sizeof(char), reg.tamNomeEstacao, fileDados) != (size_t)reg.tamNomeEstacao) {
                free(nomeEstacao); ok = false; break;
            }
            nomeEstacao[reg.tamNomeEstacao] = '\0'; // garante terminação nula
        }

        // mesmo processo para o nome da linha
        char *nomeLinha = "";
        if (fread(&reg.tamNomeLinha, sizeof(int), 1, fileDados) != 1) {
            if (reg.tamNomeEstacao > 0) free(nomeEstacao);
            ok = false; break;
        }
        // se o campo não for nulo, aloca memória para a string, lê do arquivo e garante a terminação nula
        if (reg.tamNomeLinha > 0) {
            nomeLinha = (char*) malloc((size_t)reg.tamNomeLinha + 1);
            if (!nomeLinha) { // se falhou a alocação, libera o que já alocou e marca como erro
                if (reg.tamNomeEstacao > 0) free(nomeEstacao);
                ok = false; break;
            }
            if (fread(nomeLinha, sizeof(char), reg.tamNomeLinha, fileDados) != (size_t)reg.tamNomeLinha) { // se falhou a leitura, libera o que já alocou e marca como erro
                if (reg.tamNomeEstacao > 0) free(nomeEstacao);
                free(nomeLinha); ok = false; break;
            }
            nomeLinha[reg.tamNomeLinha] = '\0';
        }

        reg.nomeEstacao = nomeEstacao;
        reg.nomeLinha = nomeLinha;

        // atualiza os campos na struct reg com os novos valores
        for (int j = 0; j < mParesUpdate; j++) {
            if (!aplicarParEmRegistro(&reg, &paresUpdate[j])) {
                ok = false;
                break;
            }
        }

        if (ok) {
            // verifica se o registro atualizado (que é de tamanho fixo) cabe no seu respectivo lixo
            if (TAM_LIVRE_REG(reg.tamNomeEstacao, reg.tamNomeLinha) < 0) ok = false;

            // volta-se o ponteiro para o início do RRN exato e realiza a sobrescrição chamando a função escreverReg
            if (fseek(fileDados, inicioRegistro, SEEK_SET) != 0) {
                ok = false;
            } else {
                escreverReg(fileDados, &reg);
            }
        }

        // libera a memória alocada para os campos de string antes de passar para o próximo registro, para evitar vazamentos
        if (reg.tamNomeEstacao > 0) free(reg.nomeEstacao);
        if (reg.tamNomeLinha > 0) free(reg.nomeLinha);

        if (!ok) break;
    }

    return ok;
}