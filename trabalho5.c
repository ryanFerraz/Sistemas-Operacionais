#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 4   
#define M 23 

int javalis_na_mesa = M;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_cozinheiro = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_gaules = PTHREAD_COND_INITIALIZER;
int cozinheiro_dormindo = 1;


void* Gaules(void* arg) {
    char nome = *(char*)arg;
    int id = nome - 'a';

    while (1) {
        pthread_mutex_lock(&mutex);

        while (javalis_na_mesa == 0) {
            if (cozinheiro_dormindo) {
                printf("Gaules %c(%d) acordou cozinheiro\n", nome, id);
                cozinheiro_dormindo = 0;
                pthread_cond_signal(&cond_cozinheiro);
            }
            pthread_cond_wait(&cond_gaules, &mutex);
        }

        javalis_na_mesa--;
        printf("Gaules %c(%d) comendo\n", nome, id);

        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}


void* Cozinheiro(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (javalis_na_mesa > 0)
            pthread_cond_wait(&cond_cozinheiro, &mutex);

        printf("Cozinheiro preparando %d javalis...\n", M);
        sleep(5);
        javalis_na_mesa = M;
        cozinheiro_dormindo = 1;
        pthread_cond_broadcast(&cond_gaules);
        pthread_mutex_unlock(&mutex);
    }
}


int main() {
    pthread_t t_gaules[N], t_cozinheiro;
    char nomes[N] = {'r', 'y', 'a', 'n'};

    pthread_create(&t_cozinheiro, NULL, Cozinheiro, NULL);
    for (int i = 0; i < N; i++)
        pthread_create(&t_gaules[i], NULL, Gaules, &nomes[i]);

    pthread_join(t_cozinheiro, NULL);
    for (int i = 0; i < N; i++)
        pthread_join(t_gaules[i], NULL);

    return 0;
}
