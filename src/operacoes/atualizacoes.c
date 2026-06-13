#include "../../headers/operacoes/atualizacoes.h"
#include "../../headers/operacoes/buscas.h"
#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool update(char *arquivoEntrada, char *arquivoSaida, CampoValor *paresBusca, int mParesBusca, CampoValor *paresUpdate, int mParesUpdate) {
    // processo de escrita, abre em r+b
    FILE *file = fopen(arquivoEntrada, "r+b");
    if (!file) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    char status;
    int topo;
    // lê o status e o topo da lista de removidos do cabeçalho
    if (fread(&status, sizeof(char), 1, file) != 1 || status != '1' || fread(&topo, sizeof(int), 1, file) != 1) {
        printf("Falha no processamento do arquivo.\n");

        // evita abrir o mesmo arquivo de novo ao usar o FILE* já aberto
        atualizarStatus(file, '0', true);

        fclose(file);
        return false;
    }

    // altera o status para inconsistente durante as modificações
    atualizarStatus(file, '0', true);
    bool ok = true;

    int rrn = -1;
    while(true){
        // busca o próximo registro que satisfaz os critérios para atualização
        // +1 para não ficar retornando sempre o mesmo rrn
        // seek = false, porque o ponteiro já estará no início do próximo rrn ao final do loop
        rrn = selectWhere(file, NULL, paresBusca, mParesBusca, rrn+1, true, false);
        if (rrn < 0) break;

        // cálculo do byte offset exato do registro
        long inicioRegistro = (long)TAM_CABECALHO + (long)rrn * (long)TAM_REG;
        
        if (fseek(file, inicioRegistro, SEEK_SET) != 0) { 
            ok = false; 
            break; 
        }

        // lê a flag de removido para garantir que o registro não foi marcado como removido entre a busca e a atualização
        char removido;
        if (fread(&removido, sizeof(char), 1, file) != 1) { 
            ok = false; 
            break; 
        }

        Registro reg;
        reg.removido = '0';

        // leitura dos campos do registro original
        if (fread(&reg.proximo, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.codEstacao, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.codLinha, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.codProxEstacao, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.distProxEstacao, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.codLinhaIntegra, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.codEstIntegra, sizeof(int), 1, file) != 1) { ok = false; break; }

        // leitura dos campos de string, alocando dinamicamente e tratando os casos de campos nulos
        char *nomeEstacao = "";
        if (fread(&reg.tamNomeEstacao, sizeof(int), 1, file) != 1) { ok = false; break; }
        // se o campo não for nulo, aloca memória para a string, lê do arquivo e garante a terminação nula
        if (reg.tamNomeEstacao > 0) {
            nomeEstacao = (char*) malloc((size_t)reg.tamNomeEstacao + 1);
            if (!nomeEstacao) { ok = false; break; }
            if (fread(nomeEstacao, sizeof(char), reg.tamNomeEstacao, file) != (size_t)reg.tamNomeEstacao) {
                free(nomeEstacao); ok = false; break;
            }
            nomeEstacao[reg.tamNomeEstacao] = '\0'; // garante terminação nula
        }

        // mesmo processo para o nome da linha
        char *nomeLinha = "";
        if (fread(&reg.tamNomeLinha, sizeof(int), 1, file) != 1) {
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
            if (fread(nomeLinha, sizeof(char), reg.tamNomeLinha, file) != (size_t)reg.tamNomeLinha) { // se falhou a leitura, libera o que já alocou e marca como erro
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
            if (fseek(file, inicioRegistro, SEEK_SET) != 0) {
                ok = false;
            } else {
                escreverReg(file, &reg);
            }
        }

        // libera a memória alocada para os campos de string antes de passar para o próximo registro, para evitar vazamentos
        if (reg.tamNomeEstacao > 0) free(reg.nomeEstacao);
        if (reg.tamNomeLinha > 0) free(reg.nomeLinha);

        if (!ok) break;
    }

    // se falhou no meio, trata o fechamento
    if (!ok) {
        printf("Falha no processamento do arquivo.\n");

        // o status já está '0' desde o começo, mas mantém explícito sem reabrir
        atualizarStatus(file, '0', true);

        fclose(file);
        return false;
    }

    // fim do processo de UPDATE
    atualizarStatus(file, '1', true);
    fclose(file);

    return true;
}