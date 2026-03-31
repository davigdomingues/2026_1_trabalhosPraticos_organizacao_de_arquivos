#include "operacoes.h"
#include "registro.h"
#include "cabecalho.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fornecidas.h"
#include "c-hashmap/map.h" //usando uma biblioteca, por enquanto. créditos da biblioteca para Mashpoe.

int freeMapKeys(const void* key, size_t ksize, uintptr_t value, void* usr){
    free((void*) key);
    return 0;
}

// utilizada após remoções, para garantir que os contadores estejam corretos sem a necessidade de realmente excluir os registros do arquivo
void recalcularContadores(FILE *file) {
    // recria os hashmaps para contar as estações e pares de estações únicas, varrendo o arquivo e lendo os registros um por um
    hashmap *mapEstacoes = hashmap_create();
    hashmap *mapParesEstacoes = hashmap_create();

    long posOriginal = ftell(file); // salva onde o ponteiro estava

    fseek(file, TAM_CABECALHO, SEEK_SET); // pula o cabeçalho (17 bytes)
    char removido;
    
    while(fread(&removido, sizeof(char), 1, file)) {
        if (removido == '1') {
            fseek(file, TAM_REG - 1, SEEK_CUR); // pula o resto do registro removido
            continue;
        }
        
        int codEstacao, codProxEstacao, tamNomeEstacao, tamNomeLinha;
        
        fseek(file, 4, SEEK_CUR); // pula próximo
        fread(&codEstacao, sizeof(int), 1, file);
        fseek(file, 4, SEEK_CUR); // pula codLinha
        fread(&codProxEstacao, sizeof(int), 1, file);
        fseek(file, 12, SEEK_CUR); // pula distProxEstacao, codLinhaIntegra, codEstIntegra
        
        fread(&tamNomeEstacao, sizeof(int), 1, file);
        char *nomeEstacao = NULL;
        if (tamNomeEstacao > 0) {
            nomeEstacao = (char*) malloc((size_t)tamNomeEstacao + 1);
            fread(nomeEstacao, sizeof(char), tamNomeEstacao, file);
            nomeEstacao[tamNomeEstacao] = '\0';
            hashmap_set(mapEstacoes, nomeEstacao, tamNomeEstacao + 1, codEstacao);
        }
        
        fread(&tamNomeLinha, sizeof(int), 1, file);
        fseek(file, tamNomeLinha, SEEK_CUR); // pula o nomeLinha (não precisamos dele)
        
        if (codProxEstacao != -1) {
            char *par = (char*) malloc(20);
            snprintf(par, 20, "%d-%d", codEstacao, codProxEstacao);
            hashmap_set(mapParesEstacoes, par, strlen(par) + 1, codProxEstacao);
        }

        // pula o lixo ($) para ir para o próximo registro
        int tamRestante = TAM_REG - 37 - tamNomeEstacao - tamNomeLinha;
        if (tamRestante > 0) fseek(file, tamRestante, SEEK_CUR);
    }

    int nroEstacoes = hashmap_size(mapEstacoes);
    int nroPares = hashmap_size(mapParesEstacoes);

    // escreve os novos contadores ajustados no cabeçalho
    fseek(file, 9, SEEK_SET); // o byte offset 9 é exatamente onde começa o nroEstacoes
    fwrite(&nroEstacoes, sizeof(int), 1, file);
    fwrite(&nroPares, sizeof(int), 1, file);

    // libera a memória
    hashmap_iterate(mapEstacoes, freeMapKeys, NULL);
    hashmap_free(mapEstacoes);
    hashmap_iterate(mapParesEstacoes, freeMapKeys, NULL);
    hashmap_free(mapParesEstacoes);

    fseek(file, posOriginal, SEEK_SET); // devolve o ponteiro de arquivo para onde estava
}

static int encontrarIndexCampo(CampoValor *pares, int mPares, const char *campo) {
    for (int i = 0; i < mPares; i++) {
        if (strcmp(pares[i].campo, campo) == 0) return i;
    }
    return -1;
}

int create(char *arquivoEntrada, char *arquivoSaida){
    //tenta abrir para leitura+escrita
    FILE *file = fopen(arquivoSaida, "rb+");
    //se não existir, cria o arquivo
    if(!file) file = fopen(arquivoSaida, "wb");

    inicializarCabecalho(file);

    int proxRRN = 0;
    //cria um hashmap para depois obter, eficientemente, o nroEstacoes únicas
    hashmap *mapEstacoes = hashmap_create();
    //cria um hashmap para depois obter, eficientemente, o nroParesEstacoes únicas
    hashmap *mapParesEstacoes = hashmap_create();

    FILE *csv = fopen(arquivoEntrada, "r");
    if(!csv){
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    char *linha = (char*) malloc(105 * sizeof(char));
    fgets(linha, 105, csv); //ignora linha de nomes das colunas
    while(fgets(linha, 105, csv) != NULL){
        char *linhaPtr = linha;

        //campos que garantidamente não são nulos
        int codEstacao = atoi(strsep(&linhaPtr, ","));
        char *nomeEstacao = strsep(&linhaPtr, ",");

        Registro *reg = (Registro*) malloc(sizeof(Registro));
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
            char *par = (char*) malloc(sizeof(char) * 10);
            //constrói uma string para representar unicamente o par
            snprintf(par, 10, "%d-%d", reg->codEstacao, reg->codProxEstacao);
            //salva no hashmap. o valor salvo não importa
            hashmap_set(mapParesEstacoes, par, 10, reg->codProxEstacao);
        }

        proxRRN++;
    }

    atualizarStatus(file, '0', true);
    atualizarProxRRN(file, proxRRN, true);
    atualizarNroEstacoes(file, hashmap_size(mapEstacoes), false); //false porque nroEstacoes é o campo seguinte de proxRRN
    atualizarNroParesEstacoes(file, hashmap_size(mapParesEstacoes), false); //false porque nroParesEstacoes é o campo seguinte de nroEstacoes

    hashmap_iterate(mapEstacoes, freeMapKeys, NULL);
    hashmap_free(mapEstacoes);
    hashmap_iterate(mapParesEstacoes, freeMapKeys, NULL);
    hashmap_free(mapParesEstacoes);

    fclose(csv);
    fclose(file);
    return 0;
}

void selectAll(char *arquivoEntrada){
    FILE *file = fopen(arquivoEntrada, "rb");
    if(!file){
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    fseek(file, TAM_CABECALHO, SEEK_SET); 

    Registro *reg = (Registro*) malloc(sizeof(Registro));
    int regLidos = 0;
    char removido;
    while(fread(&removido, sizeof(char), 1, file)){
        if(removido == '1') {
            fseek(file, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

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
            char *nomeEstacao = (char*) malloc( sizeof(char) * reg->tamNomeEstacao);
            fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
            nomeEstacao[reg->tamNomeEstacao] = '\0';
            reg->nomeEstacao = nomeEstacao;
        }

        fread(&reg->tamNomeLinha, sizeof(int), 1, file);
        if(reg->tamNomeLinha != 0){
            char *nomeLinha = (char*) malloc(sizeof(char) * reg->tamNomeLinha);
            fread(nomeLinha, sizeof(char), reg->tamNomeLinha, file);
            nomeLinha[reg->tamNomeLinha] = '\0';
            reg->nomeLinha = nomeLinha;
        }
        regLidos++;

        printReg(reg);
        int tamRestante = TAM_REG - 9 * sizeof(int) - sizeof(char) - reg->tamNomeEstacao - reg->tamNomeLinha;
        if(tamRestante != 0) fseek(file, tamRestante, SEEK_CUR); //pula os $

        free(reg->nomeEstacao);
        free(reg->nomeLinha);
    }

    if(regLidos == 0) printf("Registro inexistente.\n");
    free(reg);
}

bool verificarMatchInt(int index, char *valorQuery, int valorReg) {
    //a query inclui um valor a ser buscado para esse campo?
    if (index > -1) {
        //o valor a ser buscado para esse campo é nulo e o valor do registro atual também é?
        //ou o valor a ser buscado para esse campo não é nulo e é igual ao valor do registro atual?
        if ((*valorQuery == '\0' && valorReg == -1) || atoi(valorQuery) == valorReg) {
            //se sim, houve um match
            return true;
        }
    }
    //se não, não houve match
    return false;
}

bool verificarMatchStr(int index, char *valorQuery, char *valorReg) {
    //a query inclui um valor a ser buscado para esse campo
    //e os valores são iguais?
    if (index > -1 && strcmp(valorQuery, valorReg) == 0) { 
        //OBS: a comparação também funciona pros casos nulos
        //porque, simplesmente, a comparação é feita com strings vazias
        return true;
    }
    //se não, não houve match
    return false;
}

Registro *selectWhere(char *arquivoEntrada, CampoValor *pares, int mPares, int *tamResultados, int **rrns){
    FILE *file = fopen(arquivoEntrada, "rb");
    if(!file){
        printf("Falha no processamento do arquivo.\n");
        *tamResultados = -1;
        return NULL;
    }

    //indica em qual posição está o par campo-valor em questão
    //e, consequentemente, se a busca inclui um valor a ser buscado
    //para esse campo
    int indexCodEstacao = -1;
    int indexNomeEstacao = -1;
    int indexCodLinha = -1;
    int indexNomeLinha = -1;
    int indexCodProxEstacao = -1;
    int indexDistProxEstacao = -1;
    int indexCodLinhaIntegra = -1;
    int indexCodEstIntegra = -1;
    for (int i = 0; i < mPares; i++) {
        if(strcmp(pares[i].campo, "codEstacao") == 0){
            indexCodEstacao = i;
        } if(strcmp(pares[i].campo, "nomeEstacao") == 0){
            indexNomeEstacao = i;
        } if(strcmp(pares[i].campo, "codLinha") == 0){
            indexCodLinha = i;
        } if(strcmp(pares[i].campo, "nomeLinha") == 0){
            indexNomeLinha = i;
        } if(strcmp(pares[i].campo, "codProxEstacao") == 0){
            indexCodProxEstacao = i;
        } if(strcmp(pares[i].campo, "distProxEstacao") == 0){
            indexDistProxEstacao = i;
        } if(strcmp(pares[i].campo, "codLinhaIntegra") == 0){
            indexCodLinhaIntegra = i;
        } if(strcmp(pares[i].campo, "codEstIntegra") == 0){
            indexCodEstIntegra = i;
        }
    }

    int maxResultadosTam = 200;
    Registro *resultados = (Registro*) malloc(sizeof(Registro) * maxResultadosTam);
    int resIndexLivre = 0;

    int rrnAtual = 0;
    int *arrayRRNs = (int*) malloc(sizeof(int) * maxResultadosTam);

    int numMatches = 0;
    Registro *reg = (Registro*) malloc(sizeof(Registro));

    char removido;
    fseek(file, TAM_CABECALHO, SEEK_SET); //pula o cabeçalho
    while(fread(&removido, sizeof(char), 1, file)){
        if(removido == '1') {
            fseek(file, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            rrnAtual++;
            continue;
        }

        fseek(file, 4, SEEK_CUR); //pula os 4 bytes de proxRRN

        fread(&reg->codEstacao, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodEstacao, pares[indexCodEstacao].valor, reg->codEstacao)) numMatches++;

        fread(&reg->codLinha, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodLinha, pares[indexCodLinha].valor, reg->codLinha)) numMatches++;

        fread(&reg->codProxEstacao, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodProxEstacao, pares[indexCodProxEstacao].valor, reg->codProxEstacao)) numMatches++;

        fread(&reg->distProxEstacao, sizeof(int), 1, file);
        if (verificarMatchInt(indexDistProxEstacao, pares[indexDistProxEstacao].valor, reg->distProxEstacao)) numMatches++;

        fread(&reg->codLinhaIntegra, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodLinhaIntegra, pares[indexCodLinhaIntegra].valor, reg->codLinhaIntegra)) numMatches++;

        fread(&reg->codEstIntegra, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodEstIntegra, pares[indexCodEstIntegra].valor, reg->codEstIntegra)) numMatches++;

        fread(&reg->tamNomeEstacao, sizeof(int), 1, file);
        //lê o nomeEstacao, se não for um campo NULO
        if(reg->tamNomeEstacao != 0){
            char *nomeEstacao = (char*) malloc( sizeof(char) * reg->tamNomeEstacao);
            fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
            nomeEstacao[reg->tamNomeEstacao] = '\0';
            reg->nomeEstacao = nomeEstacao;
        } else {
            //se for NULO, só indica que é
            reg->nomeEstacao = "";
        }
        if(verificarMatchStr(indexNomeEstacao, pares[indexNomeEstacao].valor, reg->nomeEstacao)) numMatches++;

        fread(&reg->tamNomeLinha, sizeof(int), 1, file);
        //lê o nomeLinha, se não for um campo NULO
        if(reg->tamNomeLinha != 0){
            char *nomeLinha = (char*) malloc(sizeof(char) * reg->tamNomeLinha);
            fread(nomeLinha, sizeof(char), reg->tamNomeLinha, file);
            nomeLinha[reg->tamNomeLinha] = '\0';
            reg->nomeLinha = nomeLinha;
        } else {
            //se for NULO, só indica que é
            reg->nomeLinha = "";
        }
        if(verificarMatchStr(indexNomeLinha, pares[indexNomeLinha].valor, reg->nomeLinha)) numMatches++;

        if(numMatches == mPares) {
            //dobra o tamanho do array de resultados se chegou ao fim 
            if(resIndexLivre >= maxResultadosTam){
                maxResultadosTam *= 2;
                resultados = (Registro*) realloc(resultados, maxResultadosTam * sizeof(Registro));
                arrayRRNs = realloc(arrayRRNs, maxResultadosTam * sizeof(int));
            }

            //faz uma copia dos valores diretos
            Registro novoResultado;
            novoResultado = *reg;
    
            //copia as strings
            if (reg->tamNomeEstacao > 0) {
                resultados[resIndexLivre].nomeEstacao = strdup(reg->nomeEstacao);
            }
            if (reg->tamNomeLinha > 0) {
                resultados[resIndexLivre].nomeLinha = strdup(reg->nomeLinha);
            }
    
            resultados[resIndexLivre] = novoResultado;
            arrayRRNs[resIndexLivre] = rrnAtual;
            resIndexLivre++;
        }
        int tamRestante = TAM_REG - 9 * sizeof(int) - sizeof(char) - reg->tamNomeEstacao - reg->tamNomeLinha;
        if(tamRestante != 0) fseek(file, tamRestante, SEEK_CUR); //pula os $

        numMatches = 0;
        rrnAtual++;
    }
    fclose(file);
    *tamResultados = resIndexLivre;
    *rrns = arrayRRNs;
    return resultados;
}

bool deleteWhere(char *arquivoEntrada, CampoValor *pares, int mPares){
    int tamResultados = 0;
    int *arrayRRNs = NULL;

    // primeiro, obtém os registros que devem ser removidos, para depois ir no arquivo e marcar cada um deles como removido
    Registro *resultados = selectWhere(arquivoEntrada, pares, mPares, &tamResultados, &arrayRRNs);
    if (tamResultados < 0) return false;

    if (tamResultados == 0) {
        free(resultados);
        free(arrayRRNs);
        return true;
    }

    // para cada registro que deve ser removido, acessa-se o arquivo e se marca como removido, além de atualizar a lista de removidos e os contadores do cabeçalho
    FILE *file = fopen(arquivoEntrada, "r+b");
    if(!file){
        printf("Falha no processamento do arquivo.\n");
        for (int i = 0; i < tamResultados; i++) {
            if (resultados[i].tamNomeEstacao > 0) free(resultados[i].nomeEstacao);
            if (resultados[i].tamNomeLinha > 0) free(resultados[i].nomeLinha);
        }
        free(resultados);
        free(arrayRRNs);
        return false;
    }

    char status;
    int topo;

    // lê o status e o topo da lista de removidos do cabeçalho
    if (fread(&status, sizeof(char), 1, file) != 1 || status != '1' || fread(&topo, sizeof(int), 1, file) != 1) {
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        for (int i = 0; i < tamResultados; i++) {
            if (resultados[i].tamNomeEstacao > 0) free(resultados[i].nomeEstacao);
            if (resultados[i].tamNomeLinha > 0) free(resultados[i].nomeLinha);
        }
        free(resultados);
        free(arrayRRNs);
        return false;
    }

    atualizarStatus(file, '1', true); // atualiza o status para '1' para indicar que o arquivo está sendo modificado

    bool ok = true;
    for (int i = 0; i < tamResultados; i++) {
        long inicioRegistro = (long)TAM_CABECALHO + (long)arrayRRNs[i] * (long)TAM_REG;
        if (fseek(file, inicioRegistro, SEEK_SET) != 0) { 
            ok = false; break; 
        }
        char removidoFlag = '1';

        if (fwrite(&removidoFlag, sizeof(char), 1, file) != 1) { 
            ok = false; break; 
        }
        
        if (fwrite(&topo, sizeof(int), 1, file) != 1) { 
            ok = false; break; 
        }
        topo = arrayRRNs[i];
    }

    // se todas as marcações de removido foram feitas com sucesso, atualiza o topo da lista de removidos no cabeçalho para apontar para o primeiro registro removido
    if (ok) {
        if (fseek(file, 1, SEEK_SET) != 0 || fwrite(&topo, sizeof(int), 1, file) != 1) ok = false;
    }

    if (!ok) {
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        for (int i = 0; i < tamResultados; i++) {
            if (resultados[i].tamNomeEstacao > 0) free(resultados[i].nomeEstacao);
            if (resultados[i].tamNomeLinha > 0) free(resultados[i].nomeLinha);
        }
        free(resultados);
        free(arrayRRNs);
        return false;
    }

    recalcularContadores(file); // atualiza os contadores de estações e pares de estações no cabeçalho após as remoções

    fseek(file, 0, SEEK_SET);
    atualizarStatus(file, '0', false); // atualiza o status para '0' para indicar que o arquivo está consistente novamente
    fclose(file);

    for (int i = 0; i < tamResultados; i++) {
        if (resultados[i].tamNomeEstacao > 0) free(resultados[i].nomeEstacao);
        if (resultados[i].tamNomeLinha > 0) free(resultados[i].nomeLinha);
    }
    free(resultados);
    free(arrayRRNs);
    return true;
}

