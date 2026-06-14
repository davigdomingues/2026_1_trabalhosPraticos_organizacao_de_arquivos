#include "../../headers/operacoes/criacoes.h"
#include "../../headers/operacoes/insercoes.h"
#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/fornecidas.h"
#include "../../c-hashmap/map.h" //usando uma biblioteca, créditos para Mashpoe.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleCreateIndex(){
    char *arquivoDados = NULL;
    char *arquivoSaida = NULL;
    char *arquivoIndice = NULL;
    FILE *fileDados = NULL;
    FILE *fileIndice = NULL;
    bool ok = false;

    arquivoDados = lerNomeArquivo();
    if (!arquivoDados) return;

    arquivoIndice = lerNomeArquivo();
    if (!arquivoIndice){
        free(arquivoDados);
        return;
    }

    fileDados = fopen(arquivoDados, "rb");
    fileIndice = fopen(arquivoIndice, "wb+");

    if (!fileDados || !fileIndice) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados);
        free(arquivoIndice);
        if (fileDados) fclose(fileDados);
        if (fileIndice) fclose(fileIndice);
        return;
    }

    // Leitura padrão do status do arquivo de dados
    char statusDadosCreate;
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET); // Garante que a leitura do status seja feita no local correto do cabeçalho
    if (fread(&statusDadosCreate, sizeof(char), 1, fileDados) != 1 || statusDadosCreate != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados); fclose(fileIndice);
        free(arquivoDados);
        free(arquivoIndice);
        return;
    }

    ok = criarIndiceArvoreB(fileDados, fileIndice);

    // Escrita padrão do status do arquivo de índice
    if (ok) {
        char statusConsistente = '1';
        fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET);
        fwrite(&statusConsistente, sizeof(char), 1, fileIndice);
    }
    fclose(fileDados);
    fclose(fileIndice);
    if (ok) BinarioNaTela(arquivoIndice);
    free(arquivoDados);
    free(arquivoIndice);

}

void handleCreate(){
    char *arquivoDados = NULL;
    char *arquivoSaida = NULL;
    FILE *fileDados = NULL;
    FILE *fileIndice = NULL;
    bool ok = false;

    arquivoDados = lerNomeArquivo();
    if (!arquivoDados) return;

    arquivoSaida = lerNomeArquivo();
    if (!arquivoSaida) {
        free(arquivoDados);
        return;
    }

    FILE *fileCsv = fopen(arquivoDados, "r");
    FILE *fileBin = fopen(arquivoSaida, "wb+");
    if (!fileCsv || !fileBin) {
        printf("Falha no processamento do arquivo.\n");
        if (fileCsv) fclose(fileCsv);
        if (fileBin) fclose(fileBin);
        free(arquivoDados);
        free(arquivoSaida);
        return;
    }

    // Executa a carga útil de parseamento e gravação
    ok = create(fileCsv, fileBin);

    // Consolida a consistência do arquivo gerado
    if (ok) {
        char statusConsistente = '1';
        fseek(fileBin, DADOS_OFF_STATUS, SEEK_SET);
        fwrite(&statusConsistente, sizeof(char), 1, fileBin);
    }
    fclose(fileCsv);
    fclose(fileBin);
    BinarioNaTela(arquivoSaida);

    free(arquivoDados);
    free(arquivoSaida);
}

bool create(FILE *csv, FILE *fileBin) {
    int proxRRN = 0;
    //cria um hashmap para depois obter, eficientemente, o nroEstacoes únicas
    hashmap *mapEstacoes = hashmap_create();
    //cria um hashmap para depois obter, eficientemente, o nroParesEstacoes únicas
    hashmap *mapParesEstacoes = hashmap_create();

    // Inicializa as estruturas básicas de metadados no início do arquivo de dados
    inicializarCabecalho(fileBin);

    // Aloca uma string para ler as linhas do CSV
    char *linha = (char*) malloc(105 * sizeof(char));
    if (!linha) {
        hashmap_free(mapEstacoes);
        hashmap_free(mapParesEstacoes);
        return false;
    }

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
            fclose(fileBin);
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
        escreverReg(fileBin, reg);

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
        free(reg);
    }

    // Atualização dos contadores estruturais no cabeçalho do arquivo de dados
    fseek(fileBin, DADOS_OFF_PROXRRN, SEEK_SET);
    fwrite(&proxRRN, sizeof(int), 1, fileBin);

    int nroEstacoes = hashmap_size(mapEstacoes);
    int nroPares = hashmap_size(mapParesEstacoes);
    fseek(fileBin, DADOS_OFF_NROESTACOES, SEEK_SET);
    fwrite(&nroEstacoes, sizeof(int), 1, fileBin);
    fwrite(&nroPares, sizeof(int), 1, fileBin);

    // Liberações de memória locais
    hashmap_free(mapEstacoes);
    hashmap_clear(mapParesEstacoes);
    hashmap_free(mapParesEstacoes);
    free(linha);

    return true;
}

bool criarIndiceArvoreB(FILE *fileDados, FILE *fileIndice) {
    char statusDados;
    bool ok = true;
    long offsetRegistro;

    // Verifica se os arquivos foram abertos corretamente
    if(!fileDados || !fileIndice) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    // Inicializa o arquivo de índice com um cabeçalho válido, mas com status '0' (inconsistente)
    if(!escreverCabecalhoIndice(fileIndice, '0', -1, -1, 0, 0)) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    // Move o ponteiro do arquivo de dados para o início dos registros, ignorando o cabeçalho
    if(fseek(fileDados, TAM_CABECALHO, SEEK_SET) != 0) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    offsetRegistro = TAM_CABECALHO;

    int nroNos = 0;
    while (1) { // Lê cada registro do arquivo de dados e insere a chave e o ponteiro no arquivo de índice
        char removido;
        int lixo, chave, tamNomeEstacao, tamNomeLinha;
        int i;

        // Lê o campo de remoção para verificar se o registro está marcado como removido
        if (fread(&removido, sizeof(char), 1, fileDados) != 1) break;

        if (removido == '1') { // Se removido, pula para o próximo registro
            if (fseek(fileDados, TAM_REG - 1, SEEK_CUR) != 0) { // Subtrai 1 do tamanho total do registro
                ok = false;
                break;
            }
            offsetRegistro += TAM_REG;
            continue;
        }

        // Lê o campo de chave (codEstacao) para inserção
        if (fread(&lixo, sizeof(int), 1, fileDados) != 1 || fread(&chave, sizeof(int), 1, fileDados) != 1) {
            ok = false;
            break;
        }

        // Pula os campos que não são necessários
        for (i = 0; i < 5; i++) {
            if (fread(&lixo, sizeof(int), 1, fileDados) != 1) {
                ok = false;
                break;
            }
        }
        if (!ok) break;

        // Lê os tamanhos dos campos de nome para pular os dados variáveis
        if (fread(&tamNomeEstacao, sizeof(int), 1, fileDados) != 1) {
            ok = false;
            break;
        }

        // Pula o campo de nome da estação
        if (tamNomeEstacao > 0 && fseek(fileDados, tamNomeEstacao, SEEK_CUR) != 0) {
            ok = false;
            break;
        }

        // Lê o tamanho do campo de nome da linha
        if (fread(&tamNomeLinha, sizeof(int), 1, fileDados) != 1) {
            ok = false;
            break;
        }

        // Pula o campo de nome da linha
        if (tamNomeLinha > 0 && fseek(fileDados, tamNomeLinha, SEEK_CUR) != 0) {
            ok = false;
            break;
        }

        // Calcula o deslocamento para o próximo registro e posiciona o ponteiro do arquivo de dados para o início do próximo registro
        i = TAM_LIVRE_REG(tamNomeEstacao, tamNomeLinha);
        if (i > 0 && fseek(fileDados, i, SEEK_CUR) != 0) {
            ok = false;
            break;
        }

        // Insere a chave e o ponteiro para o registro no arquivo de índice e incrementa o contador de nós do índice
        if (insertIndice(fileIndice, chave, (int)offsetRegistro, &nroNos) == ERRO_DE_INSERCAO) {
            ok = false;
            break;
        }

        offsetRegistro += TAM_REG;
    }

    if (ok) { // Se foi bem-sucedida, atualiza o status do arquivo de índice para '1' e o número de nós
        fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
        fwrite(&nroNos, sizeof(int), 1, fileIndice);
    }

    if (!ok) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    return true;
}