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
    int M = atoi(argv[1]);
    int num_gaules = atoi(argv[2]);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1){ perror("shm_open"); return 1; }
    if(ftruncate(shm_fd, sizeof(Mesa)) == -1){ perror("ftruncate"); return 1; }

    Mesa *mesa = mmap(NULL, sizeof(Mesa), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(mesa == MAP_FAILED){ perror("mmap"); return 1; }

    sem_t *mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
    sem_t *sem_gaules = sem_open(SEM_GAULES_NAME, O_CREAT, 0666, 0);
    sem_t *sem_cozinheiro = sem_open(SEM_COZINHEIRO_NAME, O_CREAT, 0666, 0);
    if(mutex == SEM_FAILED || sem_gaules == SEM_FAILED || sem_cozinheiro == SEM_FAILED){
        perror("sem_open"); return 1;
    }

    mesa->M = M;
    mesa->javalis = M;
    mesa->cozinheiro_dormindo = 1;
    mesa->num_gaules = num_gaules;

    printf("Cozinheiro: M=%d, num_gaules=%d\n", M, num_gaules);
    fflush(stdout);

    while(1){
        if(sem_wait(sem_cozinheiro) == -1) break;
        if(sem_wait(mutex) == -1) break;

        printf("Cozinheiro: preparando %d javalis\n", mesa->M);
        fflush(stdout);
        sleep(1);

        mesa->javalis = mesa->M;
        mesa->cozinheiro_dormindo = 1;

        for(int i=0;i<mesa->num_gaules;i++) sem_post(sem_gaules);

        sem_post(mutex);
    }

    sem_close(mutex); sem_close(sem_gaules); sem_close(sem_cozinheiro);
    munmap(mesa, sizeof(Mesa)); close(shm_fd);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_MUTEX_NAME); sem_unlink(SEM_GAULES_NAME); sem_unlink(SEM_COZINHEIRO_NAME);
    return 0;
}
