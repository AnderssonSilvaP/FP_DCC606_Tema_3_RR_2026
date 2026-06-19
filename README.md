# 📡 Alocação Dinâmica de Espectro em Redes Celulares Densas (DSA)

Este projeto foi desenvolvido para a obtenteção de nota do trabalho final da disciplina de Análise de Algoritmo do curso de Ciência da Computação na UFRR e implementa um motor de otimização combinatória para resolver o problema de **Alocação Dinâmica de Espectro (DSA)** em redes celulares densas (5G/6G e IoT). A solução é fundamentada na teoria de **Coloração Aproximada de Grafos**, onde os elementos da rede de telecomunicações são mapeados da seguinte forma:

* **Vértices (Nós):** Representam as unidades de transmissão celular (transceivers) ou estações rádio-base.


* **Arestas:** Representam a sobreposição de áreas de cobertura, gerando potencial de interferência mútua.


* **Cores:** Representam os canais ou subfaixas de frequência disponíveis.



O objetivo do motor é **minimizar o número de cores utilizadas** (número cromático), reduzindo custos operacionais e maximizando a eficiência espectral, sem violar restrições de interferência.

## 🚀 Funcionalidades e Requisitos Atendidos

* **Estruturas de Dados Manuais:** Cumprindo a restrição do projeto, o código não utiliza bibliotecas de grafos externas (como NetworkX ou BGL). Toda a manipulação topológica (listas e matrizes de adjacência) foi construída do zero em C.


* **Fase 1 - Algoritmo Construtivo Avançado:** Utiliza a heurística **DSATUR** baseada em graus de saturação para fornecer uma solução inicial rápida e viável.


* **Fase 2 - Meta-heurística de Refinamento:** Aplica **Simulated Annealing** (Busca Local Estocástica) para tentar reduzir o número de canais utilizados, resolvendo conflitos gerados por saltos aleatórios no espaço de busca.


* **Integração Visual (Gephi):** Exportação nativa da topologia de rede para o formato `.gml`, atribuindo visualmente as frequências (cores) às estações, permitindo a análise topológica de interferências.



---

## 🛠️ Como Compilar

O motor foi desenvolvido em linguagem C para garantir máxima performance algorítmica. Para compilar, certifique-se de ter o `gcc` instalado.

No terminal, dentro do diretório do projeto, execute:

```bash
gcc main.c -o dsa_solver -O3 -lm

```

*(A flag `-O3` garante otimização de tempo de execução, vital para a meta-heurística, e `-lm` vincula a biblioteca matemática necessária para as funções de probabilidade do Simulated Annealing).*

---

## 📥 Obtendo os Benchmarks DIMACS

Para a avaliação experimental exigida pelo projeto, utilizamos os grafos do **DIMACS Graph Coloring Benchmark**. O sistema está preparado para ingerir arquivos `.col`.

1. Acesse a base de dados: [CMU DIMACS Instances](https://mat.tepper.cmu.edu/COLOR/instances.html)
2. Baixe as instâncias recomendadas no escopo do projeto, que simulam escalonamentos e redes, como:
* `le450_5a.col` 


* `flat300_28_0.col` 


* `r250.5.col` 




3. Salve os arquivos no diretório `benchmarks/` do projeto.

---

## 💻 Como Rodar a Otimização

A execução exige dois parâmetros: a instância de rede (arquivo `.col`) e o arquivo de saída para visualização visual (`.gml`).

**Sintaxe:**

```bash
./dsa_solver <caminho_instancia.col> <caminho_saida.gml>

```

**Exemplo:**

```bash
./dsa_solver benchmarks/le450_5a.col analise_le450.gml

```

**Saída no Terminal:**
O programa informará no `stderr` as reduções de frequências obtidas pela meta-heurística e, ao final, imprimirá no `stdout` um objeto JSON com as métricas exatas de qualidade de solução e eficiência temporal (em nanossegundos) , facilitando a geração de tabelas para o artigo científico.

---

## 🕸️ Tutorial de Visualização no Gephi

O projeto requer que o resultado reflita a rede celular, onde nós são as estações e arestas a interferência. Siga os passos para renderizar o mapa de espectro no **Gephi**:

1. **Importação:** Abra o Gephi e vá em `Arquivo > Abrir...`. Selecione o arquivo `.gml` gerado pelo simulador (ex: `analise_le450.gml`). Garanta que o tipo de grafo está como "Não-Direcionado".
2. **Layout (Distribuição Geográfica):** A topologia inicial parecerá um bloco denso. Na janela "Layout" (canto inferior esquerdo), escolha **ForceAtlas 2** ou **Fruchterman Reingold** e clique em "Executar". Pare a execução quando a rede de antenas estiver bem distribuída.
3. **Coloração de Canais:**
* Vá na janela **Aparência** (canto superior esquerdo) > Aba **Nós** > Ícone de **Paleta de Cores**.
* Selecione **Partição** e, no menu suspenso, escolha o atributo **Frequencia**.
* Clique em **Aplicar**. O Gephi utilizará a distribuição de canais calculada pelo algoritmo, garantindo que estações conectadas (com interferência) possuam cores diferentes.
