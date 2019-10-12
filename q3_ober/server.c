#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "server.h"
#include "main.h"

void * acceptPayment(void * s) {
    printf("Server %d initialized\n", ((Server*)s)->uid);

    while(1) {
        sem_wait(&paymentServers);

        Rider r;
        bool found = false;

        pthread_mutex_lock(&paymentMutex);

        for(int i = 0; i < M; i++) {
            if(allRiders[i]->wantsToPay) {
                r = *allRiders[i];
                allRiders[i]->wantsToPay = false;
                found = true;
                break;
            }
        }
        pthread_mutex_unlock(&paymentMutex);

        if(!found)
            break;

        sleep(2);
        printf("Server %d has accepted a payment from Rider %d\n", ((Server *)s)->uid, r.uid);
    }

    printf("Server %d closing down\n", ((Server *)s)->uid);

    pthread_exit(0);
}

void initPaymentServer(int i) {
    Server * s = (Server *) malloc(sizeof(Server));
    s->uid = i;

    pthread_create(servers_t[i], 0, acceptPayment, s);
}
