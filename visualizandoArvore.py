# Based in https://github.com/phprestes/ArvoreB-Visualizador , adapted to our project by @bielpieroli


import struct

import matplotlib.pyplot as plt
import networkx as nx
import sys

nome_arquivo = sys.argv[1]
# nro_nos = int(input())

ORDEM = 4
HEADER_SIZE = 17
PAGE_SIZE = 53
TAM_NO = PAGE_SIZE


def ler_header(headerBin):
    # IndexHeader: char status (1 byte) + 4 ints (16 bytes).
    status, noRaiz, topo, proxRRN, nroNos = struct.unpack("<ciiii", headerBin[:HEADER_SIZE])
    return {
        "status": status.decode("ascii"),
        "noRaiz": noRaiz,
        "topo": topo,
        "proxRRN": proxRRN,
        "nroNos": nroNos,
    }


def ler_no(noBin):
    # char removido; int proximo; int tipoNo; int nroChaves; (1 char + 3 ints = 13 bytes)
    # int C0; int PR0; int C1; int PR1; int C2; int PR2; (6 ints) => 24 bytes
    # int P0; int P1; int P2; int P3; (4 ints) => 16 bytes
    # total = 13 + 24 + 16 = 53 bytes (TAM_NO)
    formato = "<ciii6i4i"
    vals = struct.unpack(formato, noBin[:TAM_NO])
    removido = vals[0].decode("ascii")

    dicNo = {
        "removido": removido,
        "proximo": vals[1],
        "tipoNo": vals[2],
        "nroChaves": vals[3],
    }

    for i in range(3):
        dicNo[f"C{i+1}"] = vals[4 + i*2]
        dicNo[f"Pr{i+1}"] = vals[4 + i*2 + 1]

    for i in range(4):
        dicNo[f"P{i+1}"] = vals[10 + i]
        
    return dicNo


def no_removido(no):
    return no.get("removido") == "1"


def construir_posicoes_ordenadas(nos, raiz, distancia_x=1.8, distancia_y=1.6):
    posicoes = {}
    proximo_x = 0

    def dfs(rrn, nivel):
        nonlocal proximo_x

        if rrn not in nos or rrn in posicoes or no_removido(nos[rrn]):
            return None

        filhos = [nos[rrn][f"P{i}"] for i in range(1, 5)]
        filhos_validos = [filho for filho in filhos if filho != -1 and filho in nos and not no_removido(nos[filho])]

        if not filhos_validos:
            posicoes[rrn] = (proximo_x * distancia_x, -nivel * distancia_y)
            proximo_x += 1
            return posicoes[rrn][0]

        faixas = []
        for filho in filhos_validos:
            faixa = dfs(filho, nivel + 1)
            if faixa is not None:
                faixas.append(faixa)

        if not faixas:
            posicoes[rrn] = (proximo_x * distancia_x, -nivel * distancia_y)
            proximo_x += 1
            return posicoes[rrn][0]

        posicoes[rrn] = ((min(faixas) + max(faixas)) / 2, -nivel * distancia_y)
        return posicoes[rrn][0]

    dfs(raiz, 0)
    return posicoes


def encontrar_nos_alcancaveis(nos, raiz):
    if raiz is None or raiz not in nos:
        return set()

    visitados = set()
    pilha = [raiz]

    while pilha:
        rrn = pilha.pop()
        if rrn in visitados or rrn not in nos or no_removido(nos[rrn]):
            continue

        visitados.add(rrn)
        for i in range(1, 5):
            filho = nos[rrn][f"P{i}"]
            if filho != -1 and filho in nos and filho not in visitados and not no_removido(nos[filho]):
                pilha.append(filho)

    return visitados


def completar_posicoes_orfas(pos, nos, alcancaveis, removidos, distancia_x=1.8, distancia_y=1.6):
    # Coloca nós não alcançáveis (potencialmente destruídos/órfãos) em uma faixa separada.
    orfos = sorted([rrn for rrn in nos if rrn not in alcancaveis and rrn not in removidos])
    if not orfos:
        return pos

    if pos:
        max_x = max(x for x, _ in pos.values())
        min_y = min(y for _, y in pos.values())
    else:
        max_x = 0.0
        min_y = 0.0

    base_x = max_x + 2.5 * distancia_x
    base_y = min_y - 2.0 * distancia_y

    for i, rrn in enumerate(orfos):
        pos[rrn] = (base_x, base_y - i * distancia_y)

    return pos


def completar_posicoes_removidos(pos, removidos, distancia_x=1.8, distancia_y=1.6):
    if not removidos:
        return pos

    if pos:
        min_x = min(x for x, _ in pos.values())
        min_y = min(y for _, y in pos.values())
    else:
        min_x = 0.0
        min_y = 0.0

    base_x = min_x - 2.5 * distancia_x
    base_y = min_y - 2.0 * distancia_y

    for i, rrn in enumerate(sorted(removidos)):
        pos[rrn] = (base_x, base_y - i * distancia_y)

    return pos


##################
G = nx.DiGraph()

with open(nome_arquivo, "rb") as f:
    headerBin = f.read(17)
    header = ler_header(headerBin)
    print("Header:", header)

    nos = {}
    for i in range(header["proxRRN"]):
        noBin = f.read(TAM_NO)
        nos[i] = ler_no(noBin)

print(nos)

for id_no, no in nos.items():
    chaves = [str(no[f"C{j}"]) for j in range(1, no["nroChaves"] + 1)]
    label = f"{id_no}\n" + " | ".join(chaves) if chaves else str(id_no)

    G.add_node(id_no, label=label)
    

    if no["tipoNo"] != -1 and not no_removido(no):
        for j in range(1, 5):  # P1..P4
            filho = no[f"P{j}"]
            if filho != -1 and filho in nos and not no_removido(nos[filho]):
                G.add_edge(id_no, filho, label=f"P{j}")

# Visualização
raiz = header["noRaiz"]
if raiz == -1 or raiz not in nos:
    raiz = next(iter(nos), None)

alcancaveis = encontrar_nos_alcancaveis(nos, raiz)
removidos = {rrn for rrn, no in nos.items() if no.get("removido") == "1"}
pos = construir_posicoes_ordenadas(nos, raiz) if raiz is not None else {}
pos = completar_posicoes_orfas(pos, nos, alcancaveis, removidos)
pos = completar_posicoes_removidos(pos, removidos)
labels = nx.get_node_attributes(G, "label")
edge_labels = nx.get_edge_attributes(G, "label")

nos_orfos = sorted([rrn for rrn in nos if rrn not in alcancaveis and rrn not in removidos])
nos_removidos = sorted(removidos)

for rrn in nos_orfos:
    chaves = [str(nos[rrn][f"C{j}"]) for j in range(1, nos[rrn]["nroChaves"] + 1)]
    if chaves:
        labels[rrn] = f"{rrn} [DESTRUIDO/ORFAO]\n" + " | ".join(chaves)
    else:
        labels[rrn] = f"{rrn} [DESTRUIDO/ORFAO]"

for rrn in nos_removidos:
    chaves = [str(nos[rrn][f"C{j}"]) for j in range(1, nos[rrn]["nroChaves"] + 1)]
    if chaves:
        labels[rrn] = f"{rrn} [REMOVIDO]\n" + " | ".join(chaves)
    else:
        labels[rrn] = f"{rrn} [REMOVIDO]"

node_colors = []
for node in G.nodes():
    no = nos[node]
    if node in nos_removidos:
        node_colors.append("khaki")  # Removido
    elif node in nos_orfos:
        node_colors.append("lightgray")  # Nó não alcançável a partir da raiz
    elif no["tipoNo"] == 0:
        node_colors.append("lightcoral")  # Raiz
    elif no["tipoNo"] == 1:
        node_colors.append("lightblue")  # Intermediário
    elif no["tipoNo"] == -1:
        node_colors.append("lightgreen")  # Folha
    else:
        node_colors.append("red")  # ERRO

plt.figure(figsize=(12, 8))
nx.draw(
    G,
    pos,
    labels=labels,
    with_labels=True,
    node_shape="s",
    node_size=2000,
    node_color=node_colors,
    arrows=True,
    font_size=10,
)

nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=9)

if nos_orfos:
    texto = "Nos marcados como [DESTRUIDO/ORFAO] nao sao alcancaveis a partir da raiz. "
    texto += "Eles podem representar paginas destruídas/logicamente removidas."
    plt.gcf().text(0.01, 0.02, texto, fontsize=9, color="dimgray")

if nos_removidos:
    texto = "Nos marcados como [REMOVIDO] estao na pilha de removidos."
    plt.gcf().text(0.01, 0.05, texto, fontsize=9, color="dimgray")

plt.title(f"Árvore B de {nome_arquivo}")
plt.axis("off")
plt.show()