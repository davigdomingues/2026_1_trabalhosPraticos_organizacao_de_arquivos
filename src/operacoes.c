#include "../headers/operacoes.h"
#include "../headers/registro.h"
#include "../headers/cabecalho.h"
#include "../headers/utils.h"
#include "../headers/cabecalho.h"
#include "../c-hashmap/map.h" //usando uma biblioteca, créditos para Mashpoe.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool create(char *arquivoEntrada, char *arquivoSaida){
    //tenta abrir para leitura+escrita
    FILE *file = fopen(arquivoSaida, "rb+");
    //se não existir, cria o arquivo
    if(!file) file = fopen(arquivoSaida, "wb");

    int proxRRN = 0;
    //cria um hashmap para depois obter, eficientemente, o nroEstacoes únicas
    hashmap *mapEstacoes = hashmap_create();
    //cria um hashmap para depois obter, eficientemente, o nroParesEstacoes únicas
    hashmap *mapParesEstacoes = hashmap_create();

    FILE *csv = fopen(arquivoEntrada, "r");
    if(!csv){
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return false;
    }

    inicializarCabecalho(file);

    char *linha = (char*) malloc(105 * sizeof(char));
    fgets(linha, 105, csv); //ignora linha de nomes das colunas
    while(fgets(linha, 105, csv) != NULL){
        char *linhaPtr = linha;

        //campos que garantidamente não são nulos
        int codEstacao = atoi(strsep(&linhaPtr, ","));
        char *nomeEstacao = strsep(&linhaPtr, ",");

        Registro *reg = (Registro*) malloc(sizeof(Registro));
        if(!reg){
            free(linha);
            fclose(csv);
            fclose(file);
            printf("Falha no processamento do arquivo.\n");
            return false;
        }

        //inicializa a struct com valores que não precisam de tratamento
        *reg = (Registro) {.removido = '0', .proximo = -1, .codEstacao = codEstacao, .tamNomeEstacao = strlen(nomeEstacao), .nomeEstacao = nomeEstacao};

        //campos possívelmente nulos
        char *codLinha = strsep(&linhaPtr, ",");
        char *nomeLinha = strsep(&linhaPtr, ",");
        char *codProxEst = strsep(&linhaPtr, ",");
        char *distanciaProxEst = strsep(&linhaPtr, ",");
        char *codLinhaIntegra = strsep(&linhaPtr, ",");
        char *codEstacaoIntegra = strsep(&linhaPtr, "\n\r"); //por ser o último da linha seus delimitadores são diferentes

        //se o campo for nulo, põe -1 como valor
        reg->codLinha = *codLinha ? atoi(codLinha) : -1;
        reg->codProxEstacao = *codProxEst ? atoi(codProxEst) : -1;
        reg->distProxEstacao = *distanciaProxEst ? atoi(distanciaProxEst) : -1;
        reg->codLinhaIntegra = *codLinhaIntegra ? atoi(codLinhaIntegra) : -1;
        reg->codEstIntegra = *codEstacaoIntegra ? atoi(codEstacaoIntegra) : -1;
        
        //se o nomeLinha for nulo, põe NULL como valor e põe o tamanho como 0
        if(*nomeLinha != '\0'){
            reg->nomeLinha = nomeLinha;
            reg->tamNomeLinha = strlen(nomeLinha);
        } else {
            reg->nomeLinha = NULL;
            reg->tamNomeLinha = 0; 
        }
        escreverReg(file, reg);

        //salva no hashmap com o nome da estação sendo a chave, para garantir unicidade
        //o valor salvo não importa
        hashmap_set(mapEstacoes, strdup(reg->nomeEstacao), reg->tamNomeEstacao+1, reg->codEstacao);

        if(reg->codProxEstacao != -1){
            int menor = (reg->codEstacao < reg->codProxEstacao) ? reg->codEstacao : reg->codProxEstacao;
            int maior = (reg->codEstacao < reg->codProxEstacao) ? reg->codProxEstacao : reg->codEstacao;

            char *par = (char*) malloc(sizeof(char) * 10);
            //constrói uma string para representar o par unicamente
            snprintf(par, 10, "%d-%d", menor, maior);

            //salva o par no hashmap
            //o valor salvo não importa
            hashmap_set(mapParesEstacoes, par, 10, reg->codProxEstacao);
        }

        proxRRN++;
    }

    atualizarStatus(file, '1', true);
    atualizarProxRRN(file, proxRRN, true);
    atualizarNroEstacoes(file, hashmap_size(mapEstacoes), false); //false porque nroEstacoes é o campo seguinte de proxRRN
    atualizarNroParesEstacoes(file, hashmap_size(mapParesEstacoes), false); //false porque nroParesEstacoes é o campo seguinte de nroEstacoes

    hashmap_iterate(mapEstacoes, freeMapKeys, NULL);
    hashmap_free(mapEstacoes);
    hashmap_iterate(mapParesEstacoes, freeMapKeys, NULL);
    hashmap_free(mapParesEstacoes);

    fclose(csv);
    fclose(file);
    return true;
}

void selectAll(char *arquivoEntrada){
    FILE *file = fopen(arquivoEntrada, "rb");
    if(!file){
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    fseek(file, TAM_CABECALHO, SEEK_SET); 

    Registro *reg = (Registro*) malloc(sizeof(Registro));
    if(!reg){
        fclose(file);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    int regLidos = 0;
    char removido;
    while(fread(&removido, sizeof(char), 1, file)){
        if(removido == '1') {
            fseek(file, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

        // garante estado limpo por iteração (evita usar ponteiros antigos quando o campo é nulo)
        reg->tamNomeEstacao = 0; reg->nomeEstacao = "";
        reg->tamNomeLinha = 0; reg->nomeLinha   = "";

        fseek(file, 4, SEEK_CUR); //pula os 4 bytes de proxRRN

        //lê os campos do registro e armazena na struct
        fread(&reg->codEstacao, sizeof(int), 1, file);
        fread(&reg->codLinha, sizeof(int), 1, file);
        fread(&reg->codProxEstacao, sizeof(int), 1, file);
        fread(&reg->distProxEstacao, sizeof(int), 1, file);
        fread(&reg->codLinhaIntegra, sizeof(int), 1, file);
        fread(&reg->codEstIntegra, sizeof(int), 1, file);

        fread(&reg->tamNomeEstacao, sizeof(int), 1, file);
        if(reg->tamNomeEstacao != 0){
            char *nomeEstacao = (char*) malloc(( sizeof(char) * reg->tamNomeEstacao ) + 1); // +1 para o caractere nulo
            if (!nomeEstacao) break;
            fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
            nomeEstacao[reg->tamNomeEstacao] = '\0';
            reg->nomeEstacao = nomeEstacao;
        }

        fread(&reg->tamNomeLinha, sizeof(int), 1, file);
        if(reg->tamNomeLinha != 0){
            char *nomeLinha = (char*) malloc((sizeof(char) * reg->tamNomeLinha ) + 1); // +1 para o caractere nulo
            if (!nomeLinha) { 
                if (reg->tamNomeEstacao > 0) 
                free(reg->nomeEstacao); 
                break; 
            }

            fread(nomeLinha, sizeof(char), reg->tamNomeLinha, file);
            nomeLinha[reg->tamNomeLinha] = '\0';
            reg->nomeLinha = nomeLinha;
        }
        regLidos++;

        printReg(reg);
        int tamRestante = TAM_LIVRE_REG(reg->tamNomeEstacao, reg->tamNomeLinha);
        if(tamRestante != 0) fseek(file, tamRestante, SEEK_CUR); //pula os $

        if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
        if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
    }

    if(regLidos == 0) printf("Registro inexistente.\n");
    free(reg);
    fclose(file);
}

int selectAllWhere(char *arquivoEntrada, CampoValor *pares, int mPares){
    FILE *file = fopen(arquivoEntrada, "rb");
    if (!file) {
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    int res = selectWhere(file, pares, mPares, 0, false, true);

    fclose(file);
    return res;
}

int selectWhere(FILE *file, CampoValor *pares, int mPares, int rrnInicial, bool apenasPrimeiroRes, bool seek){
    if(!file) return -1;

    //se seek == true, vai para o rrn de início da busca
    //se rrnInicial, precisa pular o cabeçalho
    //caso contrário, assume que o ponteiro do arquivo já está no lugar certo
    if(seek || rrnInicial == 0){
        long byteOffsetInicial = (long)TAM_CABECALHO + (long)rrnInicial * (long)TAM_REG;
        fseek(file, byteOffsetInicial, SEEK_SET);
    }

    // indexa os pares por campo (posição fixa), posições sem filtro ficam como NULL
    CampoValor *porCampo[NUM_CAMPOS_REGISTRO];
    int numFiltros = popularParesPorCampo(pares, mPares, porCampo);

    int rrnAtual = rrnInicial;
    int numMatches = 0;
    Registro *reg = (Registro*) malloc(sizeof(Registro));
    if(!reg) return -1;

    bool encontrou = false;
    char removido;
    while(fread(&removido, sizeof(char), 1, file)){
        if(removido == '1') {
            fseek(file, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            rrnAtual++;
            continue;
        }

        // garante estado limpo
        reg->tamNomeEstacao = 0; reg->nomeEstacao = "";
        reg->tamNomeLinha   = 0; reg->nomeLinha   = "";

        fseek(file, 4, SEEK_CUR); //pula os 4 bytes de proxRRN

        fread(&reg->codEstacao, sizeof(int), 1, file);
        if (porCampo[CAMPO_COD_ESTACAO] && verificarMatchInt(0, porCampo[CAMPO_COD_ESTACAO]->valor, reg->codEstacao)) numMatches++;

        fread(&reg->codLinha, sizeof(int), 1, file);
        if (porCampo[CAMPO_COD_LINHA] && verificarMatchInt(0, porCampo[CAMPO_COD_LINHA]->valor, reg->codLinha)) numMatches++;

        fread(&reg->codProxEstacao, sizeof(int), 1, file);
        if (porCampo[CAMPO_COD_PROX_ESTACAO] && verificarMatchInt(0, porCampo[CAMPO_COD_PROX_ESTACAO]->valor, reg->codProxEstacao)) numMatches++;

        fread(&reg->distProxEstacao, sizeof(int), 1, file);
        if (porCampo[CAMPO_DIST_PROX_ESTACAO] && verificarMatchInt(0, porCampo[CAMPO_DIST_PROX_ESTACAO]->valor, reg->distProxEstacao)) numMatches++;

        fread(&reg->codLinhaIntegra, sizeof(int), 1, file);
        if (porCampo[CAMPO_COD_LINHA_INTEGRA] && verificarMatchInt(0, porCampo[CAMPO_COD_LINHA_INTEGRA]->valor, reg->codLinhaIntegra)) numMatches++;

        fread(&reg->codEstIntegra, sizeof(int), 1, file);
        if (porCampo[CAMPO_COD_EST_INTEGRA] && verificarMatchInt(0, porCampo[CAMPO_COD_EST_INTEGRA]->valor, reg->codEstIntegra)) numMatches++;

        fread(&reg->tamNomeEstacao, sizeof(int), 1, file);
        //lê o nomeEstacao, se não for um campo NULO
        if(reg->tamNomeEstacao != 0){
            char *nomeEstacao = (char*) malloc(( sizeof(char) * reg->tamNomeEstacao ) + 1); // +1 para o caractere nulo
            if (!nomeEstacao) { 
                free(reg); 
                return -1; 
            }
            fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
            nomeEstacao[reg->tamNomeEstacao] = '\0';
            reg->nomeEstacao = nomeEstacao;
        } else {
            //se for NULO, só indica que é
            reg->nomeEstacao = "";
        }
        if (porCampo[CAMPO_NOME_ESTACAO] && verificarMatchStr(0, porCampo[CAMPO_NOME_ESTACAO]->valor, reg->nomeEstacao)) numMatches++;

        fread(&reg->tamNomeLinha, sizeof(int), 1, file);
        //lê o nomeLinha, se não for um campo NULO
        if(reg->tamNomeLinha != 0){
            char *nomeLinha = (char*) malloc((sizeof(char) * reg->tamNomeLinha) + 1); // +1 para o caractere nulo
            if (!nomeLinha) {
                if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
                free(reg);
                return -1;
            }
            fread(nomeLinha, sizeof(char), reg->tamNomeLinha, file);
            nomeLinha[reg->tamNomeLinha] = '\0';
            reg->nomeLinha = nomeLinha;
        } else {
            //se for NULO, só indica que é
            reg->nomeLinha = "";
        }
        if (porCampo[CAMPO_NOME_LINHA] && verificarMatchStr(0, porCampo[CAMPO_NOME_LINHA]->valor, reg->nomeLinha)) numMatches++;

        if(numMatches == numFiltros) {
            encontrou = true;
            if(apenasPrimeiroRes){
                if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
                if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
                free(reg);
                return rrnAtual;
            } else {
                printReg(reg);
            }
        }
        int tamRestante = TAM_LIVRE_REG(reg->tamNomeEstacao, reg->tamNomeLinha);
        if(tamRestante != 0) fseek(file, tamRestante, SEEK_CUR); //pula os $

        if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
        if (reg->tamNomeLinha > 0) free(reg->nomeLinha);

        numMatches = 0;
        rrnAtual++;
    }

    //só imprime na tela se buscou por todos os resultados
    if(!apenasPrimeiroRes && !encontrou) printf("Registro inexistente.\n");
    if(!apenasPrimeiroRes) printf("\n");
    free(reg);

    if(!encontrou) return -1;
    else return 0;
}

bool deleteWhere(char *arquivoEntrada, CampoValor *pares, int mPares){
    // para cada registro que deve ser removido, acessa-se o arquivo e se marca como removido, além de atualizar a lista de removidos e os contadores do cabeçalho
    FILE *file = fopen(arquivoEntrada, "r+b");
    if(!file){
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    char status;
    int topo;

    // lê o status e o topo da lista de removidos do cabeçalho
    if (fread(&status, sizeof(char), 1, file) != 1 || status != '1' || fread(&topo, sizeof(int), 1, file) != 1) {
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return false;
    }

    atualizarStatus(file, '0', true); // atualiza o status para '0' para indicar que o arquivo está sendo modificado

    int rrn = -1;
    bool ok = true;
    // loop para marcar os registros como removidos e atualizar a lista de removidos no cabeçalho
    while(true){
        //busca o próximo registro que satisfaz os critérios de remoção
        rrn = selectWhere(file, pares, mPares, rrn+1, true, true); //+1 para não ficar retornando sempre o mesmo rrn
        if(rrn < 0) break;

        long inicioRegistro = (long)TAM_CABECALHO + (long)rrn * (long)TAM_REG; // calcula o byte offset do registro a ser removido, usando o RRN para acessar diretamente
        if (fseek(file, inicioRegistro, SEEK_SET) != 0) {  // posiciona o ponteiro no início do registro a ser removido
            ok = false; break; 
        }
        char removidoFlag = '1';

        if (fwrite(&removidoFlag, sizeof(char), 1, file) != 1) {  // marca o registro como removido escrevendo '1' no byte de removido
            ok = false; break; 
        }
        
        if (fwrite(&topo, sizeof(int), 1, file) != 1) {  // escreve o antigo topo da lista de removidos no campo de proxRRN do registro removido, para manter a lista encadeada
            ok = false; break; 
        }
        topo = rrn; // atualiza o topo para o RRN do registro recém-removido, que agora é o primeiro da lista de removidos
        fseek(file, inicioRegistro + TAM_REG, SEEK_SET);
    }

    // se todas as marcações de removido foram feitas com sucesso, atualiza o topo da lista de removidos no cabeçalho para apontar para o primeiro registro removido
    if (ok) {
        if (fseek(file, 1, SEEK_SET) != 0 || fwrite(&topo, sizeof(int), 1, file) != 1) ok = false;
    }

    // caso haja algum erro durante o processo de remoção
    if (!ok) {
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return false;
    }

    recalcularContadores(file); // atualiza os contadores de estações e pares de estações no cabeçalho após as remoções

    atualizarStatus(file, '1', true); // atualiza o status para '1' para indicar que o arquivo está consistente novamente
    fclose(file);

    return true;
}

bool insert(char *arquivoEntrada, CampoValor *valores, int mValores) {
    // tenta abrir para leitura+escrita, para depois atualizar o arquivo com o novo registro inserido
    FILE *file = fopen(arquivoEntrada, "r+b");
    if (!file) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroPares;

    // lê o status, o topo da lista de removidos e os contadores do cabeçalho
    if (fread(&status, sizeof(char), 1, file) != 1 || status != '1' ||
        fread(&topo, sizeof(int), 1, file) != 1 ||
        fread(&proxRRN, sizeof(int), 1, file) != 1 ||
        fread(&nroEstacoes, sizeof(int), 1, file) != 1 ||
        fread(&nroPares, sizeof(int), 1, file) != 1) {
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return false;
    }

    // atualiza o status para '0' para indicar que o arquivo está sendo modificado e volta o ponteiro para o início do arquivo para depois atualizar os contadores no cabeçalho
    atualizarStatus(file, '0', true);

    Registro reg = inicializarReg();
    bool ok = true;

    // aplica os pares usando utilitário compartilhado (conversão + NULO + strings)
    if (!aplicarParesEmRegistro(&reg, valores, mValores)) ok = false;

    if (ok) {
        if (TAM_LIVRE_REG(reg.tamNomeEstacao, reg.tamNomeLinha) < 0) ok = false;

        // atualiza nroEstacoes somente se for um novo nome de estação (contagem por nome)
        if (reg.tamNomeEstacao > 0 && !nomeEstacaoJaExiste(file, reg.nomeEstacao, reg.tamNomeEstacao)) {
            nroEstacoes++;
        }

        // só incrementa os pares se a próxima estação for válida (não nula)
        if (reg.codProxEstacao != -1) {
            nroPares++;
        }

        if (topo != -1) {
            // Reaproveita um registro removido.
            long off = (long)TAM_CABECALHO + (long)topo * (long)TAM_REG;
            int proximoTopo = -1;

            // lê o próximo da lista de removidos para atualizar o topo depois
            if (fseek(file, off + 1, SEEK_SET) != 0 || fread(&proximoTopo, sizeof(int), 1, file) != 1) {
                ok = false;
            } else if (fseek(file, off, SEEK_SET) != 0) { // posiciona para escrever o registro no lugar do removido
                ok = false;
            } else { // escreve o registro no lugar do removido
                escreverReg(file, &reg);
                // atualiza topo para o próximo da lista de removidos
                if (fseek(file, 1, SEEK_SET) != 0 || fwrite(&proximoTopo, sizeof(int), 1, file) != 1) ok = false;
            }

        } else {
            // Sem removidos: escreve no fim (append)
            if (fseek(file, 0, SEEK_END) != 0) ok = false;

            // escreve o novo registro no fim do arquivo e incrementa proxRRN para apontar para o próximo registro a ser inserido
            if (ok) {
                escreverReg(file, &reg);
                // somente incrementa o proxRRN quando escreve no fim do arquivo
                proxRRN++;
            }
        }

        // salva o proxRRN atualizado no cabeçalho (sempre salva, pois ele pode ter mudado no else acima)
        fseek(file, 5, SEEK_SET); 
        fwrite(&proxRRN, sizeof(int), 1, file);

        // atualiza apenas os contadores do cabeçalho (sem recalcular varrendo o arquivo)
        atualizarNroEstacoes(file, nroEstacoes, true);
        atualizarNroParesEstacoes(file, nroPares, false); //false porque são campos contíguos
        
        atualizarStatus(file, '1', true);
    }

    if (reg.tamNomeEstacao > 0) free(reg.nomeEstacao);
    if (reg.tamNomeLinha > 0) free(reg.nomeLinha);

    // caso haja algum erro durante a escrita do registro ou a atualização dos contadores
    if (!ok) {
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

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
        //busca o próximo registro que satisfaz os critérios para atualização
        //+1 para não ficar retornando sempre o mesmo rrn
        //seek = false, porque o ponteiro já estará no início do próximo rrn ao final do loop
        rrn = selectWhere(file, paresBusca, mParesBusca, rrn+1, true, false); 
        if(rrn < 0) break;

        // cálculo do byte offset exato do registro
        long inicioRegistro = (long)TAM_CABECALHO + (long)rrn * (long)TAM_REG;
        
        if (fseek(file, inicioRegistro, SEEK_SET) != 0) { 
            ok = false; 
            break; 
        }

        // lê a flag de removido apenas por segurança (o selectWhere já ignorou os removidos)
        char removido;
        if (fread(&removido, sizeof(char), 1, file) != 1) { 
            ok = false; 
            break; 
        }
        if (removido == '1') continue;

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
            // verifica se o registro atualizado (que é de tamanho fixo) cabe [cite: 366] no seu respectivo lixo
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