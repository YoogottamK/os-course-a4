#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "rider.h"
#include "main.h"

const int MAX_WAIT_TIME = 10,
      MAX_RIDE_TIME = 10,
      MAX_ARRIVAL_TIME = 5;

void * handleRider(void * r);

int checkFree(Cab c);

struct timespec * getFutureTime(int seconds);

void assignCab(Rider * r) {
    if(r->cabType == POOL) {
        bool gotOne = false;

        for(int i = 0; i < N && !gotOne; i++) {
            if(allCabs[i]->state == waitState ||
                    allCabs[i]->state == onRidePoolOne) {
                int x = checkFree(*allCabs[i]);

                if(freePoolOnes > 0 && (x == 1 || x == 2)) {
                    gotOne = true;
                    r->cab = allCabs[i];
                    allCabs[i]->state = onRidePoolFull;

                    if(x == 1)
                        allCabs[i]->r2 = r;
                    else
                        allCabs[i]->r1 = r;

                    freePoolOnes--;

                    break;
                } else if(freePoolOnes == 0 && x == 0) {
                    gotOne = true;
                    r->cab = allCabs[i];
                    allCabs[i]->state = onRidePoolOne;
                    allCabs[i]->r1 = r;

                    freeCabs--;
                    freePoolOnes++;

                    break;
                }
            }
        }
    } else if(r->cabType == PREMIER) {
        bool gotOne = false;

        for(int i = 0; i < N && !gotOne; i++) {
            if(allCabs[i]->state == waitState) {
                gotOne = true;
                r->cab = allCabs[i];
                allCabs[i]->state = onRidePremier;
                allCabs[i]->r1 = r;

                freeCabs--;

                break;
            }
        }
    }
}

bool canGetPool() {
    return freePoolOnes > 0 || freeCabs > 0;
}

bool canGetPremier() {
    return freeCabs > 0;
}

bool bookCab(Rider * r) {
    pthread_mutex_lock(&accessCabs);

    if((r->cabType == POOL && canGetPool()) ||
            (r->cabType == PREMIER && canGetPremier())) {
        assignCab(r);

        pthread_mutex_unlock(&accessCabs);
    } else if((r->cabType == POOL && !canGetPool()) ||
            (r->cabType == PREMIER && !canGetPremier())) {
        printf("I MUST WAIT!!\n");
        waitingForCab[r->uid] = r->cabType;

        int condResult = pthread_cond_timedwait(&condWait[r->uid],
                &accessCabs, getFutureTime(r->waitTime));

        printf("SOMETHING HAPPENED\n");

        if(condResult) {
            waitingForCab[r->uid] = 0;
            pthread_mutex_unlock(&accessCabs);
            return false;
        }

        assignCab(r);

        pthread_mutex_unlock(&accessCabs);
    } else {
        printf("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\n");
        return false;
    }

    return true;
}

void initRider(int uid) {
    Rider * r = (Rider *) malloc(sizeof(Rider));

    r->uid = uid;
    r->rideTime = 1 + (rand() % MAX_RIDE_TIME); // 1->PREMIER, 2->POOL
    //  r->waitTime = 1 + (rand() % MAX_WAIT_TIME);
    r->waitTime = 1;
    r->arrivalTime = 1 + (rand() % MAX_ARRIVAL_TIME);
    r->cabType = (CabType) (rand() % 2) + 1;
    r->wantsToPay = 0;
    r->cab = 0;

    allRiders[uid] = r;

    pthread_create(&riders_t[uid], 0, handleRider, r);
}

void * handleRider(void * r) {
    Rider * rider = (Rider *)r;
    bool cabWasAssigned = false;

    printf("Rider %d initialized: "
            "(arrivalTime) %d,"
            "(maxWaitTime) %d,"
            "(rideTime) %d,"
            "(cabType) %s\n",
            rider->uid,
            rider->arrivalTime,
            rider->waitTime,
            rider->rideTime,
            rider->cabType == 1 ? "PREMIER" : "POOL");

    sleep(rider->arrivalTime);

    if(bookCab(r)) {
        printf("Rider %d found cab %d\n", rider->uid, rider->cab->uid);

        sleep(rider->rideTime); // rider reaching his/her destination

        printf("Rider %d ended his ride\n", rider->uid);

        pthread_mutex_lock(&accessCabs);

        if(rider->cabType == PREMIER) {
            rider->cab->state = waitState;
            rider->cab->r1 = 0;
            freeCabs++;

            for(int i = 0; i < M; i++) {
                if(waitingForCab[i] == PREMIER || waitingForCab[i] == POOL) {
                    pthread_cond_signal(&condWait[i]);
                    pthread_mutex_unlock(&accessCabs);
                    cabWasAssigned = true;
                    break;
                }
            }
        } else if(rider->cabType == POOL) {
            int x = checkFree(*(rider->cab));

            if(x == 3) {
                rider->cab->state = onRidePoolOne;
                freePoolOnes++;
            } else {
                rider->cab->state = waitState;
                freeCabs++;
            }

            if(rider->cab->r1 == rider)
                rider->cab->r1 = 0;
            else if(rider->cab->r2 == rider)
                rider->cab->r2 = 0;

            for(int i = 0; i < M; i++) {
                if(waitingForCab[i] == POOL) {
                    pthread_cond_signal(&condWait[i]);
                    pthread_mutex_unlock(&accessCabs);
                    cabWasAssigned = true;
                    break;
                }
            }
        }

        if(!cabWasAssigned)
            pthread_mutex_unlock(&accessCabs);

        printf("Rider %d wants to pay\n", rider->uid);

        allRiders[rider->uid]->wantsToPay = true;
        sem_post(&paymentServers);
    } else {
        printf("[Rider %2d]: Timed out\n", rider->uid);
    }

    printf("KILLING RIDER %d\n", rider->uid);
    pthread_exit(0);
}

int checkFree(Cab c) {
    int r = 0;

    if(!!c.r1)
        r |= 1;
    if(!!c.r2)
        r |= 2;

    return r;
}

struct timespec * getFutureTime(int seconds) {
    struct timespec * t = (struct timespec *) malloc(sizeof(struct timespec));

    clock_gettime(CLOCK_MONOTONIC_RAW, t);
    t->tv_sec += seconds;

    printf("TIME: %ld.%ld\n", t->tv_sec, t->tv_nsec);

    return t;
}
