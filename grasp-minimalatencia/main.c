#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

struct matriz {
    int numero_elementos;
    int** elementos;
};

struct nodo {
    int indice;
    int valor;
};

void ler_arquivo(struct matriz*, char[20]);
int calcular_numero_canditados();
int* mlp_2opt(struct matriz, int*);
void imprimir_caminho(int, int*);
int calcular_custo(struct matriz, int*);
void copiar_caminho(int, int*, int*);
void construir_caminho(struct matriz, int*, float, float);
int* insercao(struct matriz, int*);
int* path_relinking(struct matriz, int*, int*);
int localizar_elemento(int*, int, int, int);

void selection_sort(struct nodo *, int);
void linha();

void imprimir_matriz(int**, int);


//-----------------------------------------------------------------------------

int main(int argc, char *argv[]) {
    int *caminho, *caminho_atual, *caminho_pr;
    int custo, custo_atual;
    int i;
    int n;
    float percentual_inicial, percentual_final;
    int debug;
    time_t inicio;
    struct matriz m;
    int iteracao = 0;
    
    //inicializando o algoritimo de geração de numeros aleatorios
    //com uma semente estatica
    srand(10);
    
    ///* -- leitura de parâmetros
    //lendo o arquivo da instância
    ler_arquivo(&m, argv[1]);
    
    //lendo os parametros referentes a quantidade de execucoes
    n = atoi(argv[2]);
    
    percentual_inicial = atof(argv[3]);
    percentual_final = atof(argv[4]);
     
    debug = atoi(argv[5]);
    //*/
    
    /* -- configurações de teste
    ler_arquivo(&m, "/Users/gleissonassis/Dropbox/Mestrado/Implementações/minima-latencia/grasp-minimalatencia/instancias/dantzig42_d.txt");
    
    n = 100000;
    percentual_inicial = 0.1;
    percentual_final = 0.9;
    debug = 1;
    */
    
    
    //alocando memória para o caminho que será criado
    caminho = malloc((m.numero_elementos + 1) * sizeof(int));
    caminho_atual = malloc((m.numero_elementos + 1) * sizeof(int));
    
    construir_caminho(m, caminho, percentual_inicial, percentual_final);
    custo = calcular_custo(m, caminho);
    custo_atual = custo;
    
    //inicializando a contagem de tempo
    inicio = time(NULL);
    
    
    if(debug) {
        printf("Instancia : %s\n\n", argv[1]);
    }
    
    do {
        construir_caminho(m, caminho_atual, percentual_inicial, percentual_final);
        
        for(i = 0; i < m.numero_elementos; i++) {
            caminho_atual = mlp_2opt(m, caminho_atual);
            
            custo_atual = calcular_custo(m, caminho_atual);
            
            if(custo_atual < custo) {
                copiar_caminho(m.numero_elementos + 1, caminho_atual, caminho);
                custo = custo_atual;
                
                if(debug) {
                    printf("Custo melhorado (2-opt (%d)): %d\n", i, custo);
                    imprimir_caminho(m.numero_elementos + 1, caminho);
                    printf("Tempo gasto: %d\n", (int)(time(NULL) - inicio));
                    printf("Iteracao: %d\n\n", iteracao);
                }
            }
        }
        
        caminho_atual = insercao(m, caminho_atual);
        
        custo_atual = calcular_custo(m, caminho_atual);
        
        if(custo_atual < custo) {
            copiar_caminho(m.numero_elementos + 1, caminho_atual, caminho);
            custo = custo_atual;
            
            if(debug) {
                printf("Custo melhorado (insercao): %d\n", custo);
                imprimir_caminho(m.numero_elementos + 1, caminho);
                printf("Tempo gasto: %d\n", (int)(time(NULL) - inicio));
                printf("Iteracao: %d\n\n", iteracao);

            }
        }
        
        custo_atual = calcular_custo(m, caminho_atual);
        
        if(custo_atual > custo){
            caminho_pr = path_relinking(m, caminho, caminho_atual);
            custo_atual = calcular_custo(m, caminho_pr);
            
            if(custo_atual < custo) {
                copiar_caminho(m.numero_elementos + 1, caminho_pr, caminho);
                custo = custo_atual;
                
                if(debug) {
                    printf("Custo melhorado (PATH): %d\n", custo);
                    imprimir_caminho(m.numero_elementos+1, caminho);
                    printf("Tempo gasto: %d\n", (int)(time(NULL) - inicio));
                    printf("Iteracao: %d\n\n", iteracao);

                }
            }
        }
        
        n--;
        iteracao++;
    }while(n > 0);
    
    if(debug) {
        printf("Caminho = ");
        imprimir_caminho(m.numero_elementos + 1, caminho);
        printf("Custo = %d\n", calcular_custo(m, caminho));
        printf("Tempo gasto : %d",  (int)(time(NULL) - inicio));
    
        linha();
    } else {
        imprimir_caminho(m.numero_elementos + 1, caminho);
        printf("%d\n%d\n", custo, (int)(time(NULL) - inicio));
    }
    
    //liberando memoria para a matriz de adjacencia e os caminhos
    free(caminho);
    free(caminho_atual);
    free(m.elementos);
    
    return 0;
}

//-----------------------------------------------------------------------------

void construir_caminho(struct matriz m, int* caminho, float percentual_inicial, float percentual_final) {
    int iv, indice_selecionado, indice_selecionado2;
    int *inserido;
    struct nodo *vizinhos;
    int numero_candidatos;
    float taxa_crescimento, percentual_atual;
    
    //estabelecendo o numero de candidatos a entrar no caminho
    numero_candidatos = ceil(percentual_inicial * m.numero_elementos);
    
    taxa_crescimento = (percentual_final - percentual_inicial) / m.numero_elementos;
    
    percentual_atual = percentual_inicial + taxa_crescimento;
    
    //alocando memoria para o array que ira informar se um elemento
    //ja foi inserido no caminho ou nao e inicializando os valores
    inserido = malloc(m.numero_elementos * sizeof(int));
    for(int i = 0; i < m.numero_elementos; i++) {
        inserido[i] = FALSE;
    }
    
    //alocando memoria para o array que irá armazenar informações sobre a vizinhança
    //do elemento atual
    vizinhos = (struct nodo*) malloc((m.numero_elementos) * sizeof(struct nodo));
    
    caminho[0] = 0;
    inserido[0] = TRUE;
    
    for(int i = 0; i < m.numero_elementos; i++) {
        //indice do vizinho atual;
        iv = 0;
        
        //construindo a lista de vizinhos e seus respectivos valores
        for(int j = 0; j < m.numero_elementos; j++) {
            //nao pode ser selecionado um elemento que ja esteja no caminho
            if(!inserido[j]) {
                vizinhos[iv].indice = j;
                vizinhos[iv].valor = m.elementos[i][j];
                
                iv++;
            }
        }
        
        if(iv == 0) {
            caminho[i + 1] = 0;
            //printf("Vertice inserido no caminho: %d\n", caminho[i + 1]);
        } else {
            //ordenando a lista de vizinhos por custo da aresta
            selection_sort(vizinhos, iv);
            
            //selecionando um elemento aleatorio para entrar no caminho
            do {
                if(numero_candidatos > iv) {
                    indice_selecionado = rand() % iv;
                    indice_selecionado2 = rand() % iv;
                } else {
                    indice_selecionado = rand() % numero_candidatos;
                    indice_selecionado2 = rand() % numero_candidatos;
                }
                
                if(!inserido[vizinhos[indice_selecionado2].indice] && vizinhos[indice_selecionado].valor > vizinhos[indice_selecionado2].valor) {
                    indice_selecionado = indice_selecionado2;
                }
            } while(inserido[vizinhos[indice_selecionado].indice] == TRUE);
            caminho[i + 1] = vizinhos[indice_selecionado].indice;
            inserido[vizinhos[indice_selecionado].indice] = TRUE;
            
            //printf("Vertice inserido no caminho: %d\n", caminho[i + 1]);
            
            numero_candidatos = ceil(percentual_atual * m.numero_elementos);
            percentual_atual += taxa_crescimento;
            
        }
    }
    
    //liberando memoria alocada para os arrays
    free(inserido);
    free(vizinhos);
}

//-----------------------------------------------------------------------------

int localizar_elemento(int* caminho, int tamanho, int valor, int posicao_inicial) {
    int i = posicao_inicial;
    
    for(; i < tamanho; i++) {
        if(caminho[i] == valor) {
            return i;
        }
    }
    
    return -1;
}

//-----------------------------------------------------------------------------

int* path_relinking(struct matriz m, int* c1, int* c2) {
    int i, j;
    int custo, tmp_custo;
    int pos;
    int tmp;
    int *caminho_tmp, *caminho_resultado;
    
    caminho_resultado = malloc((m.numero_elementos + 1) * sizeof(int));
    caminho_tmp = malloc((m.numero_elementos + 1) * sizeof(int));
    
    copiar_caminho(m.numero_elementos + 1, c1, caminho_resultado);
    copiar_caminho(m.numero_elementos + 1, c2, caminho_tmp);
    
    custo = calcular_custo(m, c1);
    
    for(i = 1; i < m.numero_elementos; i++) {
        if(c1[i] == caminho_tmp[i]) {
            continue;
        }
        
        pos = localizar_elemento(caminho_tmp, m.numero_elementos, c1[i], i + 1);
        
        for(j = pos; j > i; j--) {
            tmp = caminho_tmp[j];
            caminho_tmp[j] = caminho_tmp[j - 1];
            caminho_tmp[j - 1] = tmp;
            
            tmp_custo = calcular_custo(m, caminho_tmp);
            
            if(tmp_custo < custo) {
                custo = tmp_custo;
                copiar_caminho(m.numero_elementos + 1, caminho_tmp, caminho_resultado);
            }
        }
    }
    
    free(caminho_tmp);
    
    return caminho_resultado;
}

//-----------------------------------------------------------------------------

int* insercao(struct matriz m, int* caminho) {
    int custo, custo_insercao;
    int* caminho_resultado;
    int * caminho_tmp;
    int tmp;
    
    custo = calcular_custo(m, caminho);
    
    
    caminho_resultado = malloc((m.numero_elementos + 1) * sizeof(int));
    caminho_tmp = malloc((m.numero_elementos + 1) * sizeof(int));
    
    copiar_caminho(m.numero_elementos + 1, caminho, caminho_resultado);
    copiar_caminho(m.numero_elementos + 1, caminho, caminho_tmp);
    
    for(int i = 1; i < m.numero_elementos; i++) {
        copiar_caminho(m.numero_elementos + 1, caminho, caminho_tmp);
        
        for(int j = i + 1; j < m.numero_elementos; j++) {
            tmp = caminho_tmp[i];
            caminho_tmp[i] = caminho_tmp[j];
            caminho_tmp[j] = tmp;
            custo_insercao = calcular_custo(m, caminho_tmp);
            
            if(custo_insercao < custo) {
                custo = custo_insercao;
                copiar_caminho(m.numero_elementos + 1, caminho_tmp, caminho_resultado);
            }
        }
    }
    
    //liberando memÛria alocada para o caminho tempor·rio
    free(caminho_tmp);
    free(caminho);
    
    return caminho_resultado;
}

//-----------------------------------------------------------------------------

int* mlp_2opt(struct matriz m, int* caminho) {
    int i,j;
    int custo, custo_swap;
    int* caminho_swap;
    int * caminho_tmp;
    int tmp;
    
    custo = calcular_custo(m, caminho);
    
    caminho_swap = malloc((m.numero_elementos + 1) * sizeof(int));
    caminho_tmp = malloc((m.numero_elementos + 1) * sizeof(int));
    
    copiar_caminho(m.numero_elementos + 1, caminho, caminho_swap);
    copiar_caminho(m.numero_elementos + 1, caminho, caminho_tmp);
    
    for(i = 1; i < m.numero_elementos; i++) {
        copiar_caminho(m.numero_elementos + 1, caminho, caminho_tmp);
        
        for(j = i + 1; j < m.numero_elementos; j++) {
            tmp = caminho_tmp[i];
            caminho_tmp[i] = caminho_tmp[j];
            caminho_tmp[j] = tmp;
            custo_swap = calcular_custo(m, caminho_tmp);
            
            if(custo_swap < custo) {
                custo = custo_swap;
                copiar_caminho(m.numero_elementos + 1, caminho_tmp, caminho_swap);
            }
        }
    }
    
    //liberando memória alocada para o caminho temporário
    free(caminho_tmp);
    free(caminho);
    
    return caminho_swap;
}

//-----------------------------------------------------------------------------

void ler_arquivo(struct matriz* m, char arquivo[2000]) {
    FILE* fp;
    int t;
    
    fp = fopen(arquivo, "r");
    fscanf(fp, "%d %d\n\n", &m->numero_elementos, &t);
    
    //alocando espaço para a matriz de adjacencia
    m->elementos = malloc(m->numero_elementos * m->numero_elementos * sizeof(int));
    
    
    //pulando as linhas de 1s
    for(int i = 0; i < m->numero_elementos; i++) {
        fscanf(fp, "%d ", &t);
    }
    
    //percorrendo os elementos da matriz de ajdacencia que estao no arquivo
    for(int i = 0; i < m->numero_elementos; i++) {
        m->elementos[i] = malloc(m->numero_elementos * sizeof(int));
        
        for(int j = 0; j < m->numero_elementos; j++) {
            m->elementos[i][j] = 0;
            fscanf(fp, "%d ", &m->elementos[i][j]);
        }
    }
}

//-----------------------------------------------------------------------------

void selection_sort(struct nodo *array, int n) {
    int i, j;
    int min;
    struct nodo temp;
    
    for(i = 0; i < n - 1; i++) {
        min=i;
        for(j = i + 1; j < n; j++) {
            if(array[j].valor < array[min].valor) {
                min = j;
            }
        }
        
        temp = array[i];
        array[i] = array[min];
        array[min] = temp;
    }
}

//-----------------------------------------------------------------------------

void copiar_caminho(int n, int* origem, int* destino) {
    int i;
    
    for(i = 0; i < n; i++) {
        destino[i] = origem[i];
    }
}

//-----------------------------------------------------------------------------

int calcular_custo(struct matriz m, int* caminho) {
    int custo = 0;
    int i;
    
    for(i = 0; i < m.numero_elementos; i++) {
        custo += m.elementos[caminho[i]][caminho[i + 1]] * (m.numero_elementos - i);
    }
    
    return custo;
}

//-----------------------------------------------------------------------------

void imprimir_caminho(int n, int* caminho) {
    int i;
    
    for(i = 0; i < n; i++) {
        printf("%d ", caminho[i]);
    }
    printf("\n");
}

//-----------------------------------------------------------------------------

void linha() {
    int i;
    printf("\n");
    for(i = 0; i < 80; i++) printf("_");
    printf("\n");
}

void imprimir_matriz(int** elementos, int n) {
    linha();
    printf("Matriz\n\n");
    
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            printf("%d ", elementos[i][j]);
        }
        
        printf("\n");
    }
    
    linha();
}
