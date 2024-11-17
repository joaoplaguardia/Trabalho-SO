#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define NUM_PILOTOS 20
#define NUM_CORRIDAS 24

typedef struct {
    char nome[30];
    int carro;
    int habilidade;
    int voltas;
    int pontos;
    int posicao;
} Piloto;

typedef struct {
    char nome[20];
    int voltas;
    int resultado[20];
} Corrida;

typedef struct {
    Piloto* pilotos;
    Corrida corridas;
} Informacoes;

int posicao = 0;
pthread_mutex_t mutex;
pthread_barrier_t start_barrier;

void organiza(Piloto* pilotos){

    for (int i = 0; i < NUM_PILOTOS; i++){
        int cont = 1;
        for (int j = 0; j < NUM_PILOTOS; j++){
            if(pilotos[i].pontos < pilotos[j].pontos){
                cont++;
            }
        }
        pilotos[i].posicao = cont;
    }
    
}

void* correr(void* arg) {
    Informacoes* info = (Informacoes*)arg;
    Piloto* pilot = info->pilotos;
    Corrida etapa = info->corridas;

    int pontos[] = {25, 18, 15, 12, 10, 8, 6, 4, 2, 1};

    unsigned int seed = time(NULL) ^ pthread_self();

    pthread_barrier_wait(&start_barrier);

    while (1) {
        int aleatorio = rand_r(&seed) % 6;
        int voltas_percorridas = pilot->carro + pilot->habilidade * aleatorio;

        pthread_mutex_lock(&mutex);

        pilot->voltas += voltas_percorridas;

        if (pilot->voltas >= etapa.voltas) {
            posicao++;
            if (posicao <= 10) {
                pilot->pontos += pontos[posicao - 1];
                printf(" %d    %s - %d pontos\n", posicao, pilot->nome, pontos[posicao - 1]);
            } else {
                printf(" %d   %s - 0 pontos\n", posicao, pilot->nome);
            }
            pilot->voltas = 0;
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);

        usleep(100);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_PILOTOS];
    Piloto pilotos[NUM_PILOTOS];
    Corrida corridas[NUM_CORRIDAS];
    char linha[50];
    int i = 0;
    srand(time(NULL));

    FILE *arquivo = fopen("info.txt", "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL && i < NUM_PILOTOS + NUM_CORRIDAS) {
        if (i < NUM_PILOTOS) {
            linha[strcspn(linha, "\n")] = 0;
            strcpy(pilotos[i].nome, linha);

            fgets(linha, sizeof(linha), arquivo);
            pilotos[i].carro = atoi(linha);
            fgets(linha, sizeof(linha), arquivo);
            pilotos[i].habilidade = atoi(linha);
            pilotos[i].voltas = 0;
            pilotos[i].pontos = 0;
        } else {
            linha[strcspn(linha, "\n")] = 0;
            strcpy(corridas[i - NUM_PILOTOS].nome, linha);

            fgets(linha, sizeof(linha), arquivo);
            corridas[i - NUM_PILOTOS].voltas = atoi(linha);
        }

        i++;
    }
    fclose(arquivo);

    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&start_barrier, NULL, NUM_PILOTOS);

    for (int j = 0; j < NUM_CORRIDAS; j++) {
        printf("------------GP de %s------------\n", corridas[j].nome);
        printf("POS        NOME       PONTOS\n");

        for (i = 0; i < NUM_PILOTOS; i++) {
            Informacoes* info = malloc(sizeof(Informacoes));
            info->pilotos = &pilotos[i];
            info->corridas = corridas[j];

            pthread_create(&threads[i], NULL, correr, (void*)info);
        }

        for (int i = 0; i < NUM_PILOTOS; i++) {
            pthread_join(threads[i], NULL);
        }


        posicao = 0;
        getchar();
    }

    organiza(pilotos);

    printf("--------RESULTADO FINAL-----\n");
    printf("POS        NOME       PONTOS\n");

    for (int i = 1; i < NUM_PILOTOS; i++){
        for (int j = 0; j < NUM_PILOTOS; j++){
            if(pilotos[j].posicao == i){
                printf("%d - %s %d pontos\n", i, pilotos[j].nome, pilotos[j].pontos);
            }
        }
        
    }
    

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&start_barrier);

    return 0;
}
