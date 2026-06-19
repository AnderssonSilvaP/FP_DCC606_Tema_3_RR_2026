// Define necessário para garantir a disponibilidade do clock_gettime em POSIX
#define _POSIX_C_SOURCE 199309L 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

// --- ESTRUTURAS DO GRAFO ---
typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

typedef struct {
    int num_vertices;
    int num_edges;
    Node** adj_list;
    bool** adj_matrix; // Adicionado para lookup O(1) de arestas
    int* colors;       
    int* degrees;      
    int* sat_degrees;  
} Graph;

// --- FUNÇÕES BASE E GERENCIAMENTO DE MEMÓRIA ---
Graph* create_graph(int vertices) {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    g->num_vertices = vertices;
    g->num_edges = 0;
    
    g->adj_list = (Node**)calloc(vertices, sizeof(Node*));
    g->adj_matrix = (bool**)malloc(vertices * sizeof(bool*));
    g->colors = (int*)malloc(vertices * sizeof(int));
    g->degrees = (int*)calloc(vertices, sizeof(int));
    g->sat_degrees = (int*)calloc(vertices, sizeof(int));

    for (int i = 0; i < vertices; i++) {
        g->adj_matrix[i] = (bool*)calloc(vertices, sizeof(bool));
        g->colors[i] = -1; 
    }
    return g;
}

void add_edge(Graph* g, int src, int dest) {
    if (g->adj_matrix[src][dest]) return; 

    g->adj_matrix[src][dest] = true;
    g->adj_matrix[dest][src] = true;

    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->vertex = dest;
    new_node->next = g->adj_list[src];
    g->adj_list[src] = new_node;
    g->degrees[src]++;

    new_node = (Node*)malloc(sizeof(Node));
    new_node->vertex = src;
    new_node->next = g->adj_list[dest];
    g->adj_list[dest] = new_node;
    g->degrees[dest]++;
    
    g->num_edges++;
}

void free_graph(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->num_vertices; i++) {
        Node* temp = g->adj_list[i];
        while (temp != NULL) {
            Node* next = temp->next;
            free(temp);
            temp = next;
        }
        free(g->adj_matrix[i]);
    }
    free(g->adj_list); free(g->adj_matrix);
    free(g->colors); free(g->degrees); free(g->sat_degrees);
    free(g);
}

// --- UTILITÁRIO DE TEMPO (Nanosegundos) ---
long long get_time_ns() {
    struct timespec ts;
    // CLOCK_MONOTONIC é ideal para medir intervalos de tempo de forma precisa
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

// --- 1. PARSER DIMACS (Adaptado para filepath via argv) ---
Graph* read_dimacs_file(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;

    char line[512];
    Graph* g = NULL;
    int vertices = 0, edges = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\r' || line[0] == '\n' || line[0] == '\0') continue;

        char type[16];
        if (sscanf(line, "%15s", type) != 1) continue;

        if (strcmp(type, "c") == 0) {
            continue; 
        } 
        else if (strcmp(type, "p") == 0) {
            char format[32];
            if (sscanf(line, "%*s %31s %d %d", format, &vertices, &edges) >= 3) {
                g = create_graph(vertices);
            }
        } 
        else if ((strcmp(type, "e") == 0 || strcmp(type, "E") == 0) && g != NULL) {
            int u = 0, v = 0;
            if (sscanf(line, "%*s %d %d", &u, &v) == 2) {
                if (u >= 1 && u <= vertices && v >= 1 && v <= vertices && u != v) {
                    add_edge(g, u - 1, v - 1);
                }
            }
        }
    }
    
    fclose(file);
    return g;
}

// --- FUNÇÃO AUXILIAR: MAPEAMENTO DE CORES HEXADECIMAIS ---
void get_hex_color(int color_id, char* hex_str) {
    // Paleta de alto contraste para as primeiras 20 frequencias
    const char* palette[] = {
        "#e6194B", "#3cb44b", "#ffe119", "#4363d8", "#f58231", 
        "#911eb4", "#42d4f4", "#f032e6", "#bfef45", "#fabed4", 
        "#469990", "#dcbeff", "#9A6324", "#fffac8", "#800000", 
        "#aaffc3", "#808000", "#ffd8b1", "#000075", "#a9a9a9"
    };
    if (color_id >= 0 && color_id < 20) {
        strcpy(hex_str, palette[color_id]);
    } else {
        // Gera uma cor deterministica e consistente para IDs maiores usando dispersao de bits (hashing)
        sprintf(hex_str, "#%06X", (unsigned int)(color_id * 2654435761U) & 0xFFFFFF);
    }
}

// --- EXPORTAÇÃO GEPHI (GML) ---
void export_to_gml(Graph* g, const char* filepath) {
    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "[Erro] Nao foi possivel criar o ficheiro Gephi: %s\n", filepath);
        return;
    }
    
    fprintf(f, "graph [\n");
    fprintf(f, "  directed 0\n");
    
    char hex_color[8];
    
    // Registra as Celulas (Nos) e a Frequencia alocada (Atributo de Cor)
    for (int i = 0; i < g->num_vertices; i++) {
        get_hex_color(g->colors[i], hex_color);
        
        fprintf(f, "  node [\n");
        fprintf(f, "    id %d\n", i);
        fprintf(f, "    label \"%d\"\n", i);
        fprintf(f, "    Frequencia %d\n", g->colors[i]); 
        
        // Injeta o bloco grafico lido nativamente pelo Gephi
        fprintf(f, "    graphics [\n");
        fprintf(f, "      fill \"%s\"\n", hex_color);
        fprintf(f, "    ]\n");
        
        fprintf(f, "  ]\n");
    }
    
    // Registra as Interferencias (Arestas)
    for (int i = 0; i < g->num_vertices; i++) {
        Node* temp = g->adj_list[i];
        while (temp != NULL) {
            if (i < temp->vertex) { // Evita arestas duplicadas no ficheiro GML
                fprintf(f, "  edge [\n");
                fprintf(f, "    source %d\n", i);
                fprintf(f, "    target %d\n", temp->vertex);
                fprintf(f, "  ]\n");
            }
            temp = temp->next;
        }
    }
    
    fprintf(f, "]\n");
    fclose(f);
}

// --- 2. ALGORITMO CONSTRUTIVO (DSATUR) ---
int dsatur_coloring(Graph* g) {
    int n = g->num_vertices;
    int max_color = 0;
    
    bool** adj_colors = (bool**)malloc(n * sizeof(bool*));
    int* uncolored_degree = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        adj_colors[i] = (bool*)calloc(n, sizeof(bool));
        uncolored_degree[i] = g->degrees[i];
    }
    
    for (int count = 0; count < n; count++) {
        int u = -1, max_sat = -1, max_deg = -1;

        for (int i = 0; i < n; i++) {
            if (g->colors[i] == -1) {
                if (g->sat_degrees[i] > max_sat || 
                   (g->sat_degrees[i] == max_sat && uncolored_degree[i] > max_deg)) {
                    max_sat = g->sat_degrees[i];
                    max_deg = uncolored_degree[i];
                    u = i;
                }
            }
        }
        
        if (u == -1) break;

        int color;
        for (color = 0; color < n; color++) {
            if (!adj_colors[u][color]) break;
        }

        g->colors[u] = color;
        if (color > max_color) max_color = color;

        Node* temp = g->adj_list[u];
        while (temp != NULL) {
            int neighbor = temp->vertex;
            if (g->colors[neighbor] == -1) {
                uncolored_degree[neighbor]--;
                if (!adj_colors[neighbor][color]) {
                    adj_colors[neighbor][color] = true;
                    g->sat_degrees[neighbor]++;
                }
            }
            temp = temp->next;
        }
    }
    
    for (int i = 0; i < n; i++) free(adj_colors[i]);
    free(adj_colors); free(uncolored_degree);
    
    return max_color + 1;
}

// --- 3. META-HEURÍSTICA (Simulated Annealing) ---
int count_conflicts(Graph* g, int* test_colors) {
    int conflicts = 0;
    for (int i = 0; i < g->num_vertices; i++) {
        Node* temp = g->adj_list[i];
        while (temp != NULL) {
            if (i < temp->vertex && test_colors[i] == test_colors[temp->vertex]) {
                conflicts++;
            }
            temp = temp->next;
        }
    }
    return conflicts;
}

int stochastic_refinement(Graph* g, int current_k) {
    int n = g->num_vertices;
    int target_k = current_k - 1;
    if (target_k <= 0) return current_k;

    int* current_colors = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        if (g->colors[i] == target_k) {
            current_colors[i] = rand() % target_k;
        } else {
            current_colors[i] = g->colors[i];
        }
    }

    double temp = 10.0;
    double cooling_rate = 0.99995; 
    int max_iter = 1000000; 
    int current_conflicts = count_conflicts(g, current_colors);

    for (int iter = 0; iter < max_iter && current_conflicts > 0; iter++) {
        int u = rand() % n;
        int old_color = current_colors[u];
        int new_color = rand() % target_k;
        
        if (old_color == new_color) continue;

        int delta = 0;
        Node* curr_neighbor = g->adj_list[u];
        while (curr_neighbor != NULL) {
            if (current_colors[curr_neighbor->vertex] == old_color) delta--;
            if (current_colors[curr_neighbor->vertex] == new_color) delta++;
            curr_neighbor = curr_neighbor->next;
        }

        if (delta <= 0 || ((double)rand() / RAND_MAX) < exp(-delta / temp)) {
            current_colors[u] = new_color;
            current_conflicts += delta;
        }

        temp *= cooling_rate;
    }

    if (current_conflicts == 0) {
        // Redirecionado para stderr para não poluir o stdout (JSON)
        fprintf(stderr, "[Meta-Heuristica] Sucesso! Cores reduzidas de %d para %d.\n", current_k, target_k);
        for (int i = 0; i < n; i++) g->colors[i] = current_colors[i];
        free(current_colors);
        return target_k;
    } else {
        fprintf(stderr, "[Meta-Heuristica] Estagnacao. Melhor cenario mantido em %d cores (Conflitos restantes: %d).\n", current_k, current_conflicts);
        free(current_colors);
        return current_k;
    }
}

// --- MAIN ---
int main(int argc, char* argv[]) {
    // 1. Validação de Argumentos
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <caminho_do_arquivo.col> <caminho_saida_gephi.gml>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    // 2. Ingestão do Grafo
    Graph* network = read_dimacs_file(argv[1]);
    if (network == NULL) {
        fprintf(stderr, "Erro: Falha ao abrir ou ler o arquivo '%s'.\n", argv[1]);
        return 1;
    }

    fprintf(stderr, "[Info] Grafo processado: %d estacoes | %d arestas.\n", 
            network->num_vertices, network->num_edges); 

    // Fase 1: Construtiva (DSATUR)
    long long start_dsatur = get_time_ns();
    int k_construtivo = dsatur_coloring(network);
    long long end_dsatur = get_time_ns();
    long long time_dsatur = end_dsatur - start_dsatur;

    // Fase 2: Refinamento Estocástico (Simulated Annealing)
    long long start_sa = get_time_ns();
    int k_atual = k_construtivo, k_novo;
    
    do {
        k_novo = stochastic_refinement(network, k_atual);
        if (k_novo < k_atual) k_atual = k_novo;
    } while (k_novo < k_atual);
    
    long long end_sa = get_time_ns();
    long long time_sa = end_sa - start_sa;

    // Calcula os conflitos restantes da topologia (deve ser 0 para soluções viáveis)
    int conflitos_finais = count_conflicts(network, network->colors);

    // Exporta a topologia resolvida para visualização no Gephi
    export_to_gml(network, argv[2]);

    // 3. Saída Estruturada em JSON (Apenas no stdout)
    printf("{\n");
    printf("  \"graph\": { \"vertices\": %d, \"edges\": %d },\n", network->num_vertices, network->num_edges);
    printf("  \"dsatur\": { \"colors\": %d, \"time\": %lld },\n", k_construtivo, time_dsatur);
    printf("  \"sa\": { \"colors\": %d, \"time\": %lld, \"conflicts\": %d }\n", k_atual, time_sa, conflitos_finais);
    printf("}\n");

    free_graph(network);
    return 0;
}