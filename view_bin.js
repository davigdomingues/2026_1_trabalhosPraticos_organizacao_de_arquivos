// 1. O Gatekeeper: Valida estritamente a extensão do arquivo
registerFileType((fileExt, filePath, fileData) => { 
    // O parâmetro fileExt vem sem o ponto (ex: 'bin')
    return fileExt.toLowerCase() === 'bin'; 
}); 

// 2. O Roteador e Parser Inteligente
registerParser((filePath) => { 
    // Definição de cores do tema escuro
    setDefaults({ 
        "dark-colors": { 
            "collapse": "var(--vscode-textLink-foreground)", 
            "offset": "var(--vscode-descriptionForeground)", 
            "size": "var(--vscode-descriptionForeground)", 
            "name": "var(--vscode-foreground)", 
            "value": "var(--vscode-editorInfo-foreground)", 
            "description": "var(--vscode-descriptionForeground)", 
            "row-header": "var(--vscode-sideBar-background)", 
            "row-odd": "var(--vscode-editor-background)", 
            "row-even": "var(--vscode-sideBar-background)" 
        } 
    }); 

    // =========================================================================
    // LEITURA DO CABEÇALHO UNIFICADO (Primeiros 17 bytes de estruturas equivalentes)
    // =========================================================================
    read(1); let statusVal = getStringValue();
    read(4); let campo2 = getSignedNumberValue(); // Árvore: noRaiz          | Dados: topo
    read(4); let campo3 = getSignedNumberValue(); // Árvore: topo            | Dados: proxRRN
    read(4); let campo4 = getSignedNumberValue(); // Árvore: proxRRN         | Dados: nroEstacoes
    read(4); let campo5 = getSignedNumberValue(); // Árvore: nroNos          | Dados: nroParesEstacao

    // =========================================================================
    // HEURÍSTICA DE DETECÇÃO AUTOMÁTICA
    // =========================================================================
    // Se o campo3 for -1, indica que o topo da pilha de nós excluídos da Árvore-B está vazia.
    // Em arquivos de dados válidos, o campo3 representa 'proxRRN', sendo sempre >= 0.
    let ehArvore = false;
    if (campo3 === -1 && campo2 >= -1) {
        ehArvore = true;
    } else if (campo5 === 0 && campo4 === 0 && campo3 === 0) {
        ehArvore = true; // Fallback estrutural para índices recém-criados
    }

    // =========================================================================
    // EXECUÇÃO DO FLUXO ESPECÍFICO
    // =========================================================================
    if (ehArvore) {
        // PARSER DA ÁRVORE-B (Registros/Nós de 53 bytes)
        addRow('status', statusVal, '0 inconsistente, 1 consistente [Detectado: Árvore-B]'); 
        addRow('noRaiz', campo2, 'RRN do nó raíz'); 
        addRow('topo', campo3, 'topo da pilha de removidos'); 
        addRow('proxRRN', campo4, ''); 
        
        let nroNos = campo5; 
        addRow('nroNos', nroNos, ''); 

        for (let i = 0; i < nroNos; i++) { 
            read(53); 
            addRow(`Nó da Árvore (RRN ${i})`, ''); 
            addDetails(() => { 
                read(1);  addRow('removido', getStringValue(), ''); 
                read(4);  addRow('proximo', getSignedNumberValue(), ''); 
                read(4);  addRow('tipoNo', getSignedNumberValue(), '-1=folha, 0=raíz, 1=intermediário'); 
                read(4);  addRow('nroChaves', getSignedNumberValue(), ''); 
                read(4);  addRow('C1', getSignedNumberValue(), 'Chave 1'); 
                read(4);  addRow('Pr1', getSignedNumberValue(), 'RRN da Chave 1'); 
                read(4);  addRow('C2', getSignedNumberValue(), 'Chave 2'); 
                read(4);  addRow('Pr2', getSignedNumberValue(), 'RRN da Chave 2'); 
                read(4);  addRow('C3', getSignedNumberValue(), 'Chave 3'); 
                read(4);  addRow('Pr3', getSignedNumberValue(), 'RRN da Chave 3'); 
                read(4);  addRow('P1', getSignedNumberValue(), 'Ponteiro < C1'); 
                read(4);  addRow('P2', getSignedNumberValue(), 'Ponteiro < C2'); 
                read(4);  addRow('P3', getSignedNumberValue(), 'Ponteiro < C3'); 
                read(4);  addRow('P4', getSignedNumberValue(), 'Ponteiro > C3'); 
            }); 
        } 
    } else { 
        // PARSER DO ARQUIVO DE DADOS (Registros de tamanho máximo de 80 bytes)
        addRow('status', statusVal, '0 inconsistente, 1 consistente [Detectado: Dados]'); 
        addRow('topo', campo2, 'RRN do topo da pilha'); 
        
        let proxRRN = campo3; 
        addRow('proxRRN', proxRRN, ''); 
        addRow('nroEstacoes', campo4, ''); 
        addRow('nroParesEstacao', campo5, ''); 

        for (let i = 0; i < proxRRN; i++) { 
            read(80); 
            addRow(`Registro de Dados (RRN ${i})`, ''); 
            addDetails(() => { 
                read(1);   addRow('removido', getStringValue(), ''); 
                read(4);   addRow('proximo', getSignedNumberValue(), ''); 
                read(4);   addRow('codEstacao', getSignedNumberValue(), ''); 
                read(4);   addRow('codLinha', getSignedNumberValue(), ''); 
                read(4);   addRow('codProxEstacao', getSignedNumberValue(), ''); 
                read(4);   addRow('distProxEstacao', getSignedNumberValue(), ''); 
                read(4);   addRow('codLinhaIntegra', getSignedNumberValue(), ''); 
                read(4);   addRow('codEstIntegra', getSignedNumberValue(), ''); 
                
                read(4); 
                let tamNomeEstacao = getSignedNumberValue(); 
                addRow('tamNomeEstacao', tamNomeEstacao, ''); 
                if (tamNomeEstacao > 0) { 
                    read(tamNomeEstacao); 
                    addRow('nomeEstacao', getStringValue(), ''); 
                } 

                read(4); 
                let tamNomeLinha = getSignedNumberValue(); 
                addRow('tamNomeLinha', tamNomeLinha, ''); 
                if (tamNomeLinha > 0) { 
                    read(tamNomeLinha); 
                    addRow('nomeLinha', getStringValue(), ''); 
                } 

                let bytesLidos = 37 + tamNomeEstacao + tamNomeLinha; 
                let lixo = 80 - bytesLidos; 
                if (lixo > 0) { 
                    read(lixo); 
                    addRow('Lixo ($)', getStringValue(), `${lixo} bytes finais`); 
                } 
            }); 
        } 
    } 
});