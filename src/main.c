/*
    Integrantes:
    - Davi Gabriel Domingues (15447497)
    - Felipe Ferreira Colona (15636525)
*/

#include "../headers/dados/operacoes.h"
#include "../headers/indice/btree_operacoes.h"
#include "../headers/fornecidas.h"
#include "../headers/dados/registro.h"
#include "../headers/dados/cabecalho.h"
#include "../headers/indice/btree_cabecalho.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TAM_CAMPO 20
#define TAM_VALOR 50
#define TAM_ARQUIVO 100 // tamanho máximo para nome de arquivo
#define MAX_PARES 8 // número máximo de pares


bool calculaNroEstacoesUnicas(FILE *fileDados, int *nroEstacoes, int *nroParesEstacao){
    fseek(fileDados, TAM_CABECALHO, SEEK_SET);

    Registro *reg = (Registro*) malloc(sizeof(Registro));
    if(!reg){
        fclose(fileDados);
        printf("Falha no processamento do arquivo.\n");
        return true;
    }

    //cria um hashmap para depois obter, eficientemente, o nroEstacoes únicas
    hashmap *mapEstacoes = hashmap_create();
    //cria um hashmap para depois obter, eficientemente, o nroParesEstacoes únicas
    hashmap *mapParesEstacoes = hashmap_create();

    char removido;
    while(fread(&removido, sizeof(char), 1, fileDados)){
        if(removido == '1') {
            fseek(fileDados, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

        // garante estado limpo por iteração (evita usar ponteiros antigos quando o campo é nulo)
        reg->tamNomeEstacao = 0; reg->nomeEstacao = "";
        reg->tamNomeLinha = 0; reg->nomeLinha   = "";

        fseek(fileDados, 4, SEEK_CUR); //pula os 4 bytes de proxRRN

        //lê os campos do registro e armazena na struct
        fread(&reg->codEstacao, sizeof(int), 1, fileDados);
        fread(&reg->codLinha, sizeof(int), 1, fileDados);
        fread(&reg->codProxEstacao, sizeof(int), 1, fileDados);
        fread(&reg->distProxEstacao, sizeof(int), 1, fileDados);
        fread(&reg->codLinhaIntegra, sizeof(int), 1, fileDados);
        fread(&reg->codEstIntegra, sizeof(int), 1, fileDados);

        fread(&reg->tamNomeEstacao, sizeof(int), 1, fileDados);
        if(reg->tamNomeEstacao != 0){
            char *nomeEstacao = (char*) malloc(( sizeof(char) * reg->tamNomeEstacao ) + 1); // +1 para o caractere nulo
            if (!nomeEstacao) break;
            fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, fileDados);
            nomeEstacao[reg->tamNomeEstacao] = '\0';
            reg->nomeEstacao = nomeEstacao;
        }

        fread(&reg->tamNomeLinha, sizeof(int), 1, fileDados);
        if(reg->tamNomeLinha != 0){
            char *nomeLinha = (char*) malloc((sizeof(char) * reg->tamNomeLinha ) + 1); // +1 para o caractere nulo
            if (!nomeLinha) { 
                if (reg->tamNomeEstacao > 0) 
                    free(reg->nomeEstacao); 
                break; 
            }

            fread(nomeLinha, sizeof(char), reg->tamNomeLinha, fileDados);
            nomeLinha[reg->tamNomeLinha] = '\0';
            reg->nomeLinha = nomeLinha;


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
        }

        int tamRestante = TAM_LIVRE_REG(reg->tamNomeEstacao, reg->tamNomeLinha);
        if(tamRestante != 0) fseek(fileDados, tamRestante, SEEK_CUR); //pula os $

        if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
        if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
    }
    *nroEstacoes = hashmap_size(mapEstacoes);
    *nroParesEstacao = hashmap_size(mapParesEstacoes);

    free(reg);

    hashmap_iterate(mapEstacoes, freeMapKeys, NULL);
    hashmap_free(mapEstacoes);
    hashmap_iterate(mapParesEstacoes, freeMapKeys, NULL);
    hashmap_free(mapParesEstacoes);

    return true;
}

/** @brief lê os pares campo-valor da entrada padrão e armazená-los em um array de CampoValor
 * 
 * @param pares array de CampoValor a ser preenchido
 * @param mPares tamanho do array de pares
 */
static void lerPares(CampoValor *pares, int mPares) {
    for (int j = 0; j < mPares; j++) {
        char *campo = (char*) malloc(sizeof(char) * TAM_CAMPO);
        char *valor = (char*) malloc(sizeof(char) * TAM_VALOR);

        // evita overflow
        scanf("%19s", campo);

        int valorInt;
        if (scanf("%d", &valorInt) <= 0) {
            ScanQuoteString(valor);
        } else {
            // TAM_VALOR é o tamanho do buffer alocado
            snprintf(valor, TAM_VALOR, "%d", valorInt);
        }

        pares[j] = (CampoValor){.campo = campo, .valor = valor};
    }
}

/** @brief libera a memória alocada para os campos e os valores de um array de CampoValor
 * 
 * @param pares array de CampoValor a ter sua memória liberada
 * @param mPares tamanho do array de pares
 */
static void liberarPares(CampoValor *pares, int mPares) {
    for (int j = 0; j < mPares; j++) {
        free(pares[j].campo);
        free(pares[j].valor);
    }
}

/** @brief lê um nome de arquivo (token sem espaços) e devolve uma string alocada
 * 
 * @return ponteiro para a string alocada, ou NULL em caso de erro
 */
static char *lerNomeArquivo(void) {
    char *s = (char*) malloc(TAM_ARQUIVO); // aloca o máximo necessário para o nome do arquivo, incluindo o caractere nulo
    if (!s) return NULL;

    // uso de TAM_ARQUIVO - 1 para limitar a string, em que a forma direta é montada em runtime
    int width = TAM_ARQUIVO - 1;
    if (width < 1) { 
        free(s); 
        return NULL; 
    }

    // formatação para ler uma string sem espaços, limitada ao tamanho do buffer alocado
    char fmt[32];
    snprintf(fmt, sizeof(fmt), "%%%ds", width); // o %%%ds é usado para criar a formatação correta, resultando na leitura até TAM_ARQUIVO - 1 caracteres

    // se a leitura falhar, libera a memória e retorna NULL
    if (scanf(fmt, s) != 1) {
        free(s);
        return NULL;
    }
    return s;
}

int main(){
    int op;
    scanf("%d", &op);

    char *arquivoDados = NULL;
    char *arquivoSaida = NULL;
    char *arquivoIndice = NULL;
    FILE *fileDados = NULL;
    FILE *fileIndice = NULL;
    int nroEstacoes;
    int nroParesEstacoes;
    bool ok = false;
    switch (op) {
        case 1: // CREATE
            arquivoDados = lerNomeArquivo();
            arquivoSaida = lerNomeArquivo();
            if (!arquivoDados || !arquivoSaida){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            ok = create(arquivoDados, arquivoSaida);
            if(ok) BinarioNaTela(arquivoSaida);
            break;
        case 2: // SELECT ALL
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            fileDados = fopen(arquivoDados, "rb");
            if (!fileDados) {
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }
            selectAll(fileDados);
            break;
        case 3: // SELECT ALL WHERE
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            fileDados = fopen(arquivoDados, "rb");
            if (!fileDados) {
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            int nBuscas = 0;
            scanf("%d", &nBuscas);

            CampoValor *pares = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            for (int i = 0; i < nBuscas; i++) {
                int mPares = 0;
                scanf("%d", &mPares);

                lerPares(pares, mPares);

                selectAllWhere(fileDados, NULL, pares, mPares);

                liberarPares(pares, mPares);
            }
            free(pares);
            fclose(fileDados);
            break;
        case 4: // DELETE WHERE
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            int nRemocoes = 0; // número de operações de remoção a serem realizadas
            scanf("%d", &nRemocoes);

            CampoValor *paresDelete = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES); // pares da remoção atual
            ok = true;
            // loop operações de deleção
            for (int i = 0; i < nRemocoes; i++) {
                int mPares = 0;
                scanf("%d", &mPares);

                lerPares(paresDelete, mPares); // leitura dos pares para a operação de remoção atual

                if (ok && !deleteWhere(arquivoDados, paresDelete, mPares)) {
                    ok = false; // como falhou, não tenta as próximas
                }

                liberarPares(paresDelete, mPares); // liberação dos pares da operação de remoção atual antes de ler os próximos, para evitar acúmulo de memória alocada
            }

            free(paresDelete);

            if (ok) BinarioNaTela(arquivoDados);
            break;
        case 5: // INSERT
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            fileDados = fopen(arquivoDados, "rb+");
            if (!fileDados) {
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            int nInsercoes = 0; // número de operações de inserção a serem realizadas
            scanf("%d", &nInsercoes);

            ok = true; // se houver falha em alguma das inserções, ok = false e as próximas inserções não são tentadas

            // cada inserção inclui os valores de todos os campos do registro, mesmo que sejam nulos
            for (int i = 0; i < nInsercoes; i++) {
                CampoValor valores[MAX_PARES];
                char valoresStr[MAX_PARES][LIMITE];
                valores[0].campo = "codEstacao";
                valores[1].campo = "nomeEstacao";
                valores[2].campo = "codLinha";
                valores[3].campo = "nomeLinha";
                valores[4].campo = "codProxEstacao";
                valores[5].campo = "distProxEstacao";
                valores[6].campo = "codLinhaIntegra";
                valores[7].campo = "codEstacaoIntegra";

                // lê os valores como string, mesmo os inteiros, para padronizar
                // se o valor for nulo, salva como string vazia
                for (int k = 0; k < MAX_PARES; k++) {
                    valores[k].valor = valoresStr[k];
                    ScanQuoteString(valores[k].valor);
                }

                //reseta o ponteiro pro início do arquivo
                //para inserções sucessivas
                if(i != 0) fseek(fileDados, 0, SEEK_SET);

                // se houver falha na inserção, ok = false e as próximas inserções não são tentadas
                int dummy; //não vai ser utilizado
                if (ok && !insert(fileDados, NULL, valores, MAX_PARES, &dummy)) ok = false;
            }

            calculaNroEstacoesUnicas(fileDados, &nroEstacoes, &nroParesEstacoes);
            fseek(fileDados, 9, SEEK_SET);
            fwrite(&nroEstacoes, sizeof(int), 1, fileDados);
            fwrite(&nroParesEstacoes, sizeof(int), 1, fileDados);

            fclose(fileDados);
            if (ok) BinarioNaTela(arquivoDados);
            break;
        case 6: // UPDATE
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            int nAtualizacoes = 0; // número de operações de atualização a serem realizadas
            scanf("%d", &nAtualizacoes);

            // busca e atualização (pares campo-valor)
            CampoValor *paresBusca = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            CampoValor *paresUpdate = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            
            // loop das operações de atualização
            bool okUpdate = true;

            for (int i = 0; i < nAtualizacoes; i++) {
                int mParesBusca = 0;
                scanf("%d", &mParesBusca);

                lerPares(paresBusca, mParesBusca); // leitura dos pares para a parte de busca da operação de atualização atual

                int mParesUpdate = 0; // número de pares para a parte de atualização da operação de atualização atual
                scanf("%d", &mParesUpdate);

                lerPares(paresUpdate, mParesUpdate);

                // se houver falha em alguma das atualizações, okUpdate = false e as próximas atualizações não são tentadas
                if (okUpdate && !update(arquivoDados, arquivoDados, paresBusca, mParesBusca, paresUpdate, mParesUpdate)) {
                    okUpdate = false; // falha real
                }

                // liberação dos pares da operação de atualização atual antes de ler os próximos
                liberarPares(paresBusca, mParesBusca);
                liberarPares(paresUpdate, mParesUpdate);

                if (!okUpdate) break; // encerra antes de completar as N atualizações
            }

            free(paresBusca);
            free(paresUpdate);

            if (okUpdate) BinarioNaTela(arquivoDados);

            break;
        case 7: // CREATE INDEX ÁRVORE-B
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados) return 0;

            arquivoIndice = lerNomeArquivo();
            if (!arquivoIndice) return 0;

            fileDados = fopen(arquivoDados, "rb");
            fileIndice = fopen(arquivoIndice, "wb+");

            if (!fileDados || !fileIndice) {
                printf("Falha no processamento do arquivo.\n");
                if (fileDados) fclose(fileDados);
                if (fileIndice) fclose(fileIndice);
                break;
            }

            ok = criarIndiceArvoreB(fileDados, fileIndice);
            
            fclose(fileDados);
            fclose(fileIndice);

            if (ok) BinarioNaTela(arquivoIndice);
            break;
        case 8: //SELECT WHERE COM INDEXAÇÃO
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            arquivoIndice = lerNomeArquivo();
            if (!arquivoIndice){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            fileDados = fopen(arquivoDados, "rb");
            if (!fileDados) {
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            if(arquivoIndice != NULL){
                fileIndice = fopen(arquivoIndice, "rb");
            }

            nBuscas = 0;
            scanf("%d", &nBuscas);

            pares = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            for (int i = 0; i < nBuscas; i++) {
                int mPares = 0;
                scanf("%d", &mPares);

                lerPares(pares, mPares);

                selectAllWhere(fileDados, fileIndice, pares, mPares);
                liberarPares(pares, mPares);
            }

            free(pares);
            fclose(fileDados);
            if(fileIndice) fclose(fileIndice);
            break;
        case 9: // INSERT COM INDEXAÇÃO
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            arquivoIndice = lerNomeArquivo();
            if (!arquivoIndice){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            fileDados = fopen(arquivoDados, "rb+");
            if (!fileDados) {
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            fileIndice = fopen(arquivoIndice, "rb+");
            if(!fileIndice){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            nInsercoes = 0; // número de operações de inserção a serem realizadas
            scanf("%d", &nInsercoes);

            //lê o nroNos ao abrir o arquivo para 
            //só atualizar ao final das 'n' operações 
            int nroNos;
            if(fileIndice){
                fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
                fread(&nroNos, sizeof(int), 1, fileIndice);
            }

            // cada inserção inclui os valores de todos os campos do registro, mesmo que sejam nulos
            for (int i = 0; i < nInsercoes; i++) {
                CampoValor valores[MAX_PARES];
                char valoresStr[MAX_PARES][LIMITE];
                valores[0].campo = "codEstacao";
                valores[1].campo = "nomeEstacao";
                valores[2].campo = "codLinha";
                valores[3].campo = "nomeLinha";
                valores[4].campo = "codProxEstacao";
                valores[5].campo = "distProxEstacao";
                valores[6].campo = "codLinhaIntegra";
                valores[7].campo = "codEstacaoIntegra";

                // lê os valores como string, mesmo os inteiros, para padronizar
                // se o valor for nulo, salva como string vazia
                for (int k = 0; k < MAX_PARES; k++) {
                    valores[k].valor = valoresStr[k];

                    int n;
                    if(scanf("%d", &n)) snprintf(valores[k].valor, sizeof(valores[k].valor), "%d", n);
                    else ScanQuoteString(valores[k].valor);
                }

                //reseta o ponteiro pro início do arquivo
                //para inserções sucessivas
                if(i != 0) fseek(fileDados, 0, SEEK_SET);
                bool sucesso = insert(fileDados, fileIndice, valores, MAX_PARES, &nroNos);
            }

            calculaNroEstacoesUnicas(fileDados, &nroEstacoes, &nroParesEstacoes);
            fseek(fileDados, 9, SEEK_SET);
            fwrite(&nroEstacoes, sizeof(int), 1, fileDados);
            fwrite(&nroParesEstacoes, sizeof(int), 1, fileDados);

            //após as 'n' operações, atualiza o nroNos
            fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
            fwrite(&nroNos, sizeof(int), 1, fileIndice);

            fclose(fileDados);
            if(fileIndice) fclose(fileIndice);

            BinarioNaTela(arquivoDados);
            BinarioNaTela(arquivoIndice);
            break;
        case 10: // DELETE WHERE COM INDEXAÇÃO
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }
            
            arquivoIndice = lerNomeArquivo();
            if (!arquivoIndice){
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }

            fileDados = fopen(arquivoDados, "r+b");
            fileIndice = fopen(arquivoIndice, "r+b");

            if (!fileDados || !fileIndice) {
                printf("Falha no processamento do arquivo.\n");
                if (fileDados) fclose(fileDados);
                if (fileIndice) fclose(fileIndice);
                return 0;
            }

            int nRemocoesIdx = 0;
            scanf("%d", &nRemocoesIdx);

            CampoValor *paresDeleteIdx = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            ok = true;

            for (int i = 0; i < nRemocoesIdx; i++) {
                int mPares = 0;
                scanf("%d", &mPares);
                
                lerPares(paresDeleteIdx, mPares);

                if (ok && !deleteWhereIndexado(fileDados, fileIndice, paresDeleteIdx, mPares)) {
                    ok = false; // falha crítica em alguma parte do processo
                }

                liberarPares(paresDeleteIdx, mPares); 
            }
            free(paresDeleteIdx);

            fclose(fileDados);
            fclose(fileIndice);

            if (ok) {
                BinarioNaTela(arquivoDados);
                BinarioNaTela(arquivoIndice);
            }
            break;
        default: // operação inválida
            return -1;
    }
    free(arquivoDados);
    free(arquivoIndice);
    free(arquivoSaida);

    return 0;
}