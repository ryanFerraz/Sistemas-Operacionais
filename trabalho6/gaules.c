#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "mesa.h"

#define SHM_NAME "/mesa_shm_rferraz"
#define SEM_MUTEX_NAME "/sem_mutex_rferraz"
#define SEM_GAULES_NAME "/sem_gaules_rferraz"
#define SEM_COZINHEIRO_NAME "/sem_cozinheiro_rferraz"

int main(int argc, char **argv){
    if(argc < 3) return 1;
    char letra = argv[1][0];
    int idx = atoi(argv[2]);

    int shm_fd;
    Mesa *mesa;
    sem_t *mutex;
    sem_t *sem_gaules;
    sem_t *sem_cozinheiro;

    for(;;){
        shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if(shm_fd != -1) break;
        sleep(1);
    }

    mesa = mmap(NULL, sizeof(Mesa), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(mesa == MAP_FAILED){ perror("mmap"); return 1; }

    for(;;){
        mutex = sem_open(SEM_MUTEX_NAME, 0);
        sem_gaules = sem_open(SEM_GAULES_NAME, 0);
        sem_cozinheiro = sem_open(SEM_COZINHEIRO_NAME, 0);
        if(mutex != SEM_FAILED && sem_gaules != SEM_FAILED && sem_cozinheiro != SEM_FAILED) break;
        if(mutex != SEM_FAILED) sem_close(mutex);
        if(sem_gaules != SEM_FAILED) sem_close(sem_gaules);
        if(sem_cozinheiro != SEM_FAILED) sem_close(sem_cozinheiro);
        sleep(1);
    }

    while(1){
        sem_wait(mutex);

        if(mesa->javalis == 0){
            if(mesa->cozinheiro_dormindo){
                printf("Gaules %c(%d) acordou cozinheiro\n", letra, idx);
                fflush(stdout);
                mesa->cozinheiro_dormindo = 0;
                sem_post(sem_cozinheiro);
            }
            sem_post(mutex);
            sem_wait(sem_gaules);
            continue;
        }

        mesa->javalis--;
        printf("Gaules %c(%d) comendo (restam %d)\n", letra, idx, mesa->javalis);
        fflush(stdout);
        sem_post(mutex);
        sleep(1);
    }

    sem_close(mutex); sem_close(sem_gaules); sem_close(sem_cozinheiro);
    munmap(mesa, sizeof(Mesa)); close(shm_fd);
    return 0;
}
