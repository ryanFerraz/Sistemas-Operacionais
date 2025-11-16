#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <string.h>
#include <fcntl.h>    
#include <sys/stat.h>

#define N 4       // número de gauleses
#define M 23      // quantidade de javalis preparados

#define FILA_COZINHEIRO "/fila_cozinheiro"
#define FILA_GAULESES   "/fila_gauleses"

int javalis_na_mesa = M;
pthread_mutex_t mutex_javalis = PTHREAD_MUTEX_INITIALIZER;

mqd_t mq_cozinheiro;
mqd_t mq_gauleses;

void* Gaules(void* arg) {
    char nome = *(char*)arg;
    int id = nome - 'a';
    char msg[64];

    while (1) {
        pthread_mutex_lock(&mutex_javalis);
        if (javalis_na_mesa == 0) {
            pthread_mutex_unlock(&mutex_javalis);

            printf("Gaules %c(%d) acordou o cozinheiro!\n", nome, id);
            strcpy(msg, "ACORDA");
            mq_send(mq_cozinheiro, msg, strlen(msg)+1, 0);
            
            // Espera o cozinheiro avisar
            mq_receive(mq_gauleses, msg, sizeof(msg), NULL);
            printf("Gaules %c(%d) recebeu: %s\n", nome, id, msg);
        }

        if (javalis_na_mesa > 0) {
            javalis_na_mesa--;
            printf("Gaules %c(%d) comendo javali. Restam %d.\n", nome, id, javalis_na_mesa);
            pthread_mutex_unlock(&mutex_javalis);
        }

        sleep(1);
    }
    return NULL;
}

void* Cozinheiro(void* arg) {
    char msg[64];

    while (1) {
        // Espera mensagem dos gauleses
        mq_receive(mq_cozinheiro, msg, sizeof(msg), NULL);
        printf("Cozinheiro acordou! Mensagem: %s\n", msg);

        printf("Cozinheiro preparando %d javalis...\n", M);
        sleep(5);
        
        pthread_mutex_lock(&mutex_javalis);
        javalis_na_mesa = M;
        pthread_mutex_unlock(&mutex_javalis);

        strcpy(msg, "JAVALIS PRONTOS!");
        // Avisa todos os gauleses
        for (int i = 0; i < N; i++)
            mq_send(mq_gauleses, msg, strlen(msg)+1, 0);
    }
    return NULL;
}


int main() {
    pthread_t t_gaules[N], t_cozinheiro;
    char nomes[N] = {'r', 'y', 'a', 'n'};

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 64;

    pthread_mutex_init(&mutex_javalis, NULL);

    // Cria filas (remove se já existirem)
    mq_unlink(FILA_COZINHEIRO);
    mq_unlink(FILA_GAULESES);

    mq_cozinheiro = mq_open(FILA_COZINHEIRO, O_CREAT | O_RDWR, 0666, &attr);
    mq_gauleses = mq_open(FILA_GAULESES, O_CREAT | O_RDWR, 0666, &attr);

    // if (mq_cozinheiro == -1 || mq_gauleses == -1) {
    //     perror("Erro ao criar filas de mensagens");
    //     exit(1);
    // }

    pthread_create(&t_cozinheiro, NULL, Cozinheiro, NULL);
    for (int i = 0; i < N; i++)
        pthread_create(&t_gaules[i], NULL, Gaules, &nomes[i]);

    pthread_join(t_cozinheiro, NULL);
    for (int i = 0; i < N; i++)
        pthread_join(t_gaules[i], NULL);

    pthread_mutex_destroy(&mutex_javalis);

    mq_close(mq_cozinheiro);
    mq_close(mq_gauleses);
    mq_unlink(FILA_COZINHEIRO);
    mq_unlink(FILA_GAULESES);

    return 0;
}
