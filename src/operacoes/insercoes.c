#include "../../headers/operacoes/insercoes.h"
#include "../../headers/operacoes/buscas.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/indice/btree_no.h"
#include "../../headers/utils.h"
#include "../../headers/fornecidas.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void distribuirUniforme(FILE *fileIndice, No *no, No *novoNo, int rrnNovoNo, ElementoIndice overflowElem, ElementoIndice *promoPraCima){
    //array que inclui os 4 elementos
    //3 do nó original e 1 de overflow
    ElementoIndice elems[4];

    elems[0] = (ElementoIndice){no->C[0], no->Pr[0], no->P[1]};
    elems[1] = (ElementoIndice){no->C[1], no->Pr[1], no->P[2]};
    elems[2] = (ElementoIndice){no->C[2], no->Pr[2], no->P[3]};
    elems[3] = overflowElem;

    //não é alterado, porque essa subárvore
    //à esquerda é, garantidamente, menor
    //do que todas as chaves desse nó
    int ponteiroMaisEsquerda = no->P[0];

    //como o array já está quase inteiro ordenado,
    //com exceção da chave nova, o insertionSort
    //é a melhor escolha
    insertionSort(elems, 4);

    //os dois menores elementos ficam nesse nó
    no->C[0] = elems[0].chave;
    no->Pr[0] = elems[0].ptrDados;
    no->P[1] = elems[0].filhoDir;
    no->C[1] = elems[1].chave;
    no->Pr[1] = elems[1].ptrDados;
    no->P[2] = elems[1].filhoDir;

    //um espaço fica vago nesse nó
    no->C[2] = -1;
    no->Pr[2] = -1;
    no->P[3] = -1;
    no->P[0] = ponteiroMaisEsquerda; //o ponteiro à esquerda é restaurado
    no->nroChaves = 2;

    //dentre as duas maiores chaves, a menor
    //é promovida
    promoPraCima->chave = elems[2].chave;
    promoPraCima->ptrDados = elems[2].ptrDados;
    promoPraCima->filhoDir = rrnNovoNo;

    //a maior chave fica no novo nó
    //que é o filho à direita do nó promovido
    novoNo->C[0] = elems[3].chave;
    novoNo->Pr[0]= elems[3].ptrDados;
    novoNo->P[0] = elems[2].filhoDir;
    novoNo->P[1] = elems[3].filhoDir;

    //limpeza
    novoNo->C[1] = -1; novoNo->C[2]  = -1;
    novoNo->Pr[1] = -1; novoNo->Pr[2] = -1;
    novoNo->P[2] = -1; novoNo->P[3]  = -1;
    
    novoNo->nroChaves = 1;
    //o novoNo tem o mesmo tipo do nó mais
    //antigo, porque eles são criados no mesmo nível
    novoNo->tipoNo = no->tipoNo;
}


bool insereOrdenado(No *no, ElementoIndice elem){
    //se não tiver espaço no nó, não permite a inserção
    if(no->nroChaves >= NRO_MAX_CHAVES) return false;

    int i = no->nroChaves - 1;
    //shifta todos os elementos maiores do que a chave para a direita
    while (i >= 0 && no->C[i] > elem.chave) {
        no->C[i+1] = no->C[i];
        no->Pr[i+1] = no->Pr[i];
        no->P[i+2] = no->P[i+1]; //o filho a direita acompanha a chave
        i--;
    }

    //insere na posição correta
    int posicaoInsercao = i + 1;
    no->C[posicaoInsercao] = elem.chave;
    no->Pr[posicaoInsercao] = elem.ptrDados;
    no->P[posicaoInsercao+1] = elem.filhoDir;

    no->nroChaves++;
    return true;
}


No *split(FILE *fileIndice, No *no, ElementoIndice overflowElem, ElementoIndice *promoPraCima){
    int rrnNovoNo;
    No *novoNo = criarNo(fileIndice, &rrnNovoNo);

    //se o nó que sofreu split era raiz, houve a criação de uma nova raiz
    //e como ela é filha da nova raiz, seu tipo passa a ser intermediário
    if (no->tipoNo == NO_RAIZ) {
        no->tipoNo = NO_INTERMEDIARIO;
    } else {
        //caso contrário, só copia o tipo
        novoNo->tipoNo = no->tipoNo; 
    }

    //distribui os elementos da forma mais uniforme possível
    //entre o nó que estourou e o novo nó criado, e define o elemento a ser promovido para o pai
    distribuirUniforme(fileIndice, no, novoNo, rrnNovoNo, overflowElem, promoPraCima);
    return novoNo;
}

void criarNovaRaiz(FILE *fileIndice, int rrnRaizAtual, ElementoIndice promoDeBaixo, int *nroNos){
    int rrnNovaRaiz;
    No *novaRaiz = criarNo(fileIndice, &rrnNovaRaiz);

    //cria nova raiz e põe a raiz antiga como filha dela
    novaRaiz->nroChaves = 1;
    novaRaiz->C[0] = promoDeBaixo.chave;
    novaRaiz->Pr[0] = promoDeBaixo.ptrDados;
    novaRaiz->P[0] = rrnRaizAtual;          //filho a esquerda é a raiz antiga
    novaRaiz->P[1] = promoDeBaixo.filhoDir; //filho a direita é o nó criado no split
    novaRaiz->tipoNo = NO_RAIZ;

    //atualiza o cabeçalho do arquivo para apontar para o RRN da nova raiz
    fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET); 
    fwrite(&rrnNovaRaiz, sizeof(int), 1, fileIndice);

    //se este for ser o único nó da árvore, 
    //quer dizer que é um nó raiz e folha.
    //por convenção, o tipo é folha
    if(*nroNos == 0) novaRaiz->tipoNo = NO_FOLHA;
    (*nroNos)++;

    //salva a nova raiz no arquivo
    int inicioNovaRaiz = BTREE_NO_INICIO(rrnNovaRaiz);
    fseek(fileIndice, inicioNovaRaiz, SEEK_SET);
    escreverNo(fileIndice, novaRaiz);
        
    free(novaRaiz);
}

int insertIndice(FILE *fileIndice, int chave, int ptrDados, int *nroNos){
    //lê do cabeçalho, o rrn da raiz 
    int rrnRaiz;
    fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    ElementoIndice promoDeBaixo;
    int res = insertIndiceRec(fileIndice, chave, ptrDados, rrnRaiz, &promoDeBaixo, nroNos);
    //se houve uma promoção vinda da raiz, então precisa criar uma nova raiz
    if(res == PROMOCAO) criarNovaRaiz(fileIndice, rrnRaiz, promoDeBaixo, nroNos);

    return 1;
}

int insertIndiceRec(FILE *fileIndice, int chave, int ptrDados, int rrnNoAtual, ElementoIndice *promoPraCima, int *nroNos){
    //chegou ao final da recursão
    if(rrnNoAtual == -1){
        promoPraCima->chave = chave;
        promoPraCima->ptrDados = ptrDados;
        promoPraCima->filhoDir = -1;
        return PROMOCAO;
    }
    int inicioNo = BTREE_NO_INICIO(rrnNoAtual);
    fseek(fileIndice, inicioNo, SEEK_SET);

    //lê o nó atual
    No *no = inicializarNo();
    lerNo(fileIndice, no);

    int subArvore;
    int dummy; //não vai ser utilizado
    //busca pela chave nesse nó
    bool encontrou = encontrarChave(no, chave, &subArvore, &dummy);
    //se encontrou, aborta a inserção
    if(encontrou) {
        free(no);
        return ERRO_DE_INSERCAO;
    }

    ElementoIndice promoDeBaixo;
    int res = insertIndiceRec(fileIndice, chave, ptrDados, subArvore, &promoDeBaixo, nroNos);

    if(res == SEM_PROMOCAO || res == ERRO_DE_INSERCAO) {
        free(no);
        return res;
    }
    //há espaço para inserção no nó atual
    else if (no->nroChaves < NRO_MAX_CHAVES){
        insereOrdenado(no, promoDeBaixo);

        fseek(fileIndice, inicioNo, SEEK_SET);
        escreverNo(fileIndice, no);
        
        free(no);
        return SEM_PROMOCAO;
    } else {
        //não há espaço para inserção no nó atual
        No *novoNo = split(fileIndice, no, promoDeBaixo, promoPraCima);

        //escreve o nó atual no arquivo
        fseek(fileIndice, inicioNo, SEEK_SET);
        escreverNo(fileIndice, no);

        //escreve o novo nó no arquivo
        int inicioNovoNo = BTREE_NO_INICIO(promoPraCima->filhoDir);
        fseek(fileIndice, inicioNovoNo, SEEK_SET);
        escreverNo(fileIndice, novoNo);

        //incrementa em memória o número de nós
        //pra posteriormente escrever no cabeçalho
        (*nroNos)++;
        
        free(no);
        free(novoNo);
        
        return PROMOCAO;
    }

    return 0;
}

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

        fseek(fileDados, DADOS_OFF_PROXRRN - 1, SEEK_CUR); //pula os 4 bytes de proxRRN

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


void handleInsert(){
    char *arquivoDados = NULL;
    FILE *fileDados = NULL;
    int nroEstacoes;
    int nroParesEstacoes;
    bool ok = false;

    arquivoDados = lerNomeArquivo();
    if (!arquivoDados){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    fileDados = fopen(arquivoDados, "rb+");
    if (!fileDados) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados);
        return;
    }

    char statusDadosInsert;
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fread(&statusDadosInsert, sizeof(char), 1, fileDados);

    if (statusDadosInsert != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados); 
        free(arquivoDados); 
        return;
    }

    char status = '0';
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fwrite(&status, sizeof(char), 1, fileDados);

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
        if(i != 0) fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET);

        // se houver falha na inserção, ok = false e as próximas inserções não são tentadas
        int dummy; //não vai ser utilizado
        if (ok && !insert(fileDados, NULL, valores, MAX_PARES, &dummy)) ok = false;
    }

    calculaNroEstacoesUnicas(fileDados, &nroEstacoes, &nroParesEstacoes);
    fseek(fileDados, 9, SEEK_SET);
    fwrite(&nroEstacoes, sizeof(int), 1, fileDados);
    fwrite(&nroParesEstacoes, sizeof(int), 1, fileDados);

    status = '1';
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fwrite(&status, sizeof(char), 1, fileDados);
    fclose(fileDados);
    BinarioNaTela(arquivoDados);

    free(arquivoDados);
    return;
}

void handleInsertIndexado(){
    char *arquivoDados = NULL;
    char *arquivoIndice = NULL;
    FILE *fileDados = NULL;
    FILE *fileIndice = NULL;
    int nroEstacoes;
    int nroParesEstacoes;
    bool ok = false;
    int nInsercoes = 0;

    arquivoDados = lerNomeArquivo();
    arquivoIndice = lerNomeArquivo();
    if (!arquivoDados || !arquivoIndice){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    fileDados = fopen(arquivoDados, "rb+");
    fileIndice = fopen(arquivoIndice, "rb+");
    if (!fileDados || !fileIndice) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados);
        free(arquivoIndice);
        return;
    }

    char statusDadosInsertIndexado;
    char statusIndiceInsertIndexado;
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fread(&statusDadosInsertIndexado, sizeof(char), 1, fileDados);

    fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET);
    fread(&statusIndiceInsertIndexado, sizeof(char), 1, fileIndice);

    if (statusDadosInsertIndexado != '1' || statusIndiceInsertIndexado != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados); 
        fclose(fileIndice);
        free(arquivoDados); 
        free(arquivoIndice);
        return;
    }

    char inconsistente = '0';
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, fileDados);

    fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, fileIndice);

    nInsercoes = 0; // número de operações de inserção a serem realizadas
    scanf("%d", &nInsercoes);

    //lê o nroNos ao abrir o arquivo para 
    //só atualiza ao final das 'n' operações 
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
        if(i != 0){
            fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET);
            fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
        }
        bool sucesso = insert(fileDados, fileIndice, valores, MAX_PARES, &nroNos);
    }

    calculaNroEstacoesUnicas(fileDados, &nroEstacoes, &nroParesEstacoes);
    fseek(fileDados, DADOS_OFF_NROESTACOES, SEEK_SET);
    fwrite(&nroEstacoes, sizeof(int), 1, fileDados);
    fwrite(&nroParesEstacoes, sizeof(int), 1, fileDados);

    //após as 'n' operações, atualiza o nroNos
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    fwrite(&nroNos, sizeof(int), 1, fileIndice);

    char consistente = '1';
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fwrite(&consistente, sizeof(char), 1, fileDados);

    fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET);
    fwrite(&consistente, sizeof(char), 1, fileIndice);

    fclose(fileDados);
    if(fileIndice) fclose(fileIndice);

    BinarioNaTela(arquivoDados);
    BinarioNaTela(arquivoIndice);

    free(arquivoDados);
    free(arquivoIndice);
}

bool insert(FILE *fileDados, FILE *fileIndice, CampoValor *valores, int mValores, int *nroNos) {
    // Verifica se já existe um registro com o mesmo código de estação usando o índice
    if(*valores[CAMPO_COD_ESTACAO].valor && fileIndice != NULL){
        CampoValor *pares[8] = {NULL};
        CampoValor *apenasCodEstacao = (CampoValor*) malloc(sizeof(CampoValor));
        apenasCodEstacao->campo = valores[CAMPO_COD_ESTACAO].campo;
        apenasCodEstacao->valor = valores[CAMPO_COD_ESTACAO].valor;
        pares[0] = apenasCodEstacao;

        // Se a busca retornar um resultado com o mesmo código de estação, operação deve ser parada (duplicidade)
        int res = selectWhereIndexado(fileDados, fileIndice, pares, 1, false);
        if(res > 0){
            free(apenasCodEstacao);
            return true;
        }

        free(apenasCodEstacao);
    }

    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroPares;

    // o topo da lista de removidos e os contadores do cabeçalho
    //não lê o status, porque ele já foi lido na main
    if (fread(&topo, sizeof(int), 1, fileDados) != 1 ||
        fread(&proxRRN, sizeof(int), 1, fileDados) != 1 ||
        fread(&nroEstacoes, sizeof(int), 1, fileDados) != 1 ||
        fread(&nroPares, sizeof(int), 1, fileDados) != 1) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    Registro reg = inicializarReg();
    int rrnNovoReg = -1;
    int ponteiroDados;
    bool ok = true;

    // aplica os pares usando utilitário compartilhado (conversão + NULO + strings)
    if (!aplicarParesEmRegistro(&reg, valores, mValores)) ok = false;

    if (ok) {
        if (TAM_LIVRE_REG(reg.tamNomeEstacao, reg.tamNomeLinha) < 0) ok = false;

        if (topo != -1) {
            // Reaproveita um registro removido
            long off = (long)TAM_CABECALHO + (long)topo * (long)TAM_REG;
            ponteiroDados = off;
            rrnNovoReg = off;
            int proximoTopo = -1;

            // lê o próximo da lista de removidos para atualizar o topo depois
            if (fseek(fileDados, off + 1, SEEK_SET) != 0 || fread(&proximoTopo, sizeof(int), 1, fileDados) != 1) {
                ok = false;
            } else if (fseek(fileDados, off, SEEK_SET) != 0) { // posiciona para escrever o registro no lugar do removido
                ok = false;
            } else { // escreve o registro no lugar do removido
                escreverReg(fileDados, &reg);
                // atualiza topo para o próximo da lista de removidos
                if (fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET) != 0 || fwrite(&proximoTopo, sizeof(int), 1, fileDados) != 1) ok = false;
            }

        } else {
            // Sem removidos: escreve no fim (append)
            if (fseek(fileDados, 0, SEEK_END) != 0) ok = false;

            // escreve o novo registro no fim do arquivo e incrementa proxRRN para apontar para o próximo registro a ser inserido
            if (ok) {
                rrnNovoReg = proxRRN;
                ponteiroDados = (long)TAM_CABECALHO + (long)rrnNovoReg * (long)TAM_REG;
                escreverReg(fileDados, &reg);
                // somente incrementa o proxRRN quando escreve no fim do arquivo
                proxRRN++;
            }
        }

        // salva o proxRRN atualizado no cabeçalho (sempre salva, pois ele pode ter mudado no else acima)
        fseek(fileDados, DADOS_OFF_PROXRRN, SEEK_SET);
        fwrite(&proxRRN, sizeof(int), 1, fileDados);
    }

    if (reg.tamNomeEstacao > 0) free(reg.nomeEstacao);
    if (reg.tamNomeLinha > 0) free(reg.nomeLinha);


    // caso haja algum erro durante a escrita do registro ou a atualização dos contadores
    if (!ok) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    // lógica específica para o arquivo de índice: se ele existir, insere o par chave-ponteiro no índice
    // e trata a questão da consistência do arquivo de índice durante a escrita
    if (fileIndice != NULL) {
        if (!fileIndice) {
            ok = false;
        } else {
            insertIndice(fileIndice, reg.codEstacao, ponteiroDados, nroNos);
        }

        if (!ok) printf("Falha no processamento do arquivo.\n");
    }
    return ok;
}