#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "main.h"
#include "cab.h"
#include "server.h"
#include "rider.h"

void init() {
    srand(time(0));

    // currently unlocked
    pthread_mutex_init(&accessCabs, 0);
    pthread_mutex_init(&paymentMutex, 0);

    sem_init(&paymentServers, 0, 0);

    freePoolOnes = 0;
    freeCabs = N;

    waitingForCab = (CabType *) malloc(sizeof(int) * N);
    for(int i = 0; i < N; i++)
        waitingForCab[i] = 0;

    allCabs = (Cab **) malloc(sizeof(Cab *) * N);
    allRiders = (Rider **) malloc(sizeof(Rider *) * M);

    servers_t = (pthread_t *) malloc(sizeof(pid_t) * K);
    riders_t = (pthread_t *) malloc(sizeof(pid_t) * M);

    condWait = (pthread_cond_t *) malloc(sizeof(pthread_cond_t) * M);
    for(int i = 0; i < M; i++)
        pthread_cond_init(&condWait[i], 0);

    for(int i = 0; i < N; i++)
        initCab(i);

    for(int i = 0; i < M; i++)
        initRider(i);

    for(int i = 0; i < K; i++)
        initPaymentServer(i);
}

void destroy() {

}

int main() {
    scanf("%d %d %d", &N, &M, &K);

    init();

    for(int i = 0; i < M; i++) {
        pthread_join(riders_t[i], 0);
        printf("JOINING SOME THREAD\n");
    }

    sleep(2);

    for(int i = 0; i < K; i++) {
        printf("GOING TO KILL SERVER\n");
        sem_post(&paymentServers);
        pthread_join(servers_t[i], 0);
    }

    destroy();

    return 0;
}
