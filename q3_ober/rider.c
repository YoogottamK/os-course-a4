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

void reachDestination(Rider * r);

bool handoverCab(Rider * rider);

void assignCab(Rider * r) {
    bool gotOne = false;

    if(r->cabType == POOL) {
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
        waitingForCab[r->uid] = 0;
        assignCab(r);

        pthread_mutex_unlock(&accessCabs);
    } else if((r->cabType == POOL && !canGetPool()) ||
            (r->cabType == PREMIER && !canGetPremier())) {
        waitingForCab[r->uid] = r->cabType;

        int condResult = pthread_cond_timedwait(&condWait[r->uid],
                &accessCabs, getFutureTime(r->waitTime));

        if(condResult) {
            waitingForCab[r->uid] = 0;
            pthread_mutex_unlock(&accessCabs);
            return false;
        }

        waitingForCab[r->uid] = 0;

        assignCab(r);

        pthread_mutex_unlock(&accessCabs);
    } else {
        return false;
    }

    return true;
}

void initRider(int uid) {
    Rider * r = (Rider *) malloc(sizeof(Rider));

    r->uid = uid;
    r->rideTime = 1 + (rand() % MAX_RIDE_TIME); // 1->PREMIER, 2->POOL
    r->waitTime = 1 + (rand() % MAX_WAIT_TIME);
    r->arrivalTime = 1 + (rand() % MAX_ARRIVAL_TIME);
    r->cabType = (CabType) (rand() % 2) + 1;
    r->wantsToPay = 0;
    r->cab = 0;

    allRiders[uid] = r;

    pthread_create(riders_t[uid], 0, handleRider, r);
}

void * handleRider(void * r) {
    Rider * rider = (Rider *)r;

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
        reachDestination(r);

        if(!handoverCab(r))
            pthread_mutex_unlock(&accessCabs);

        printf("Rider %d wants to pay\n", rider->uid);

        allRiders[rider->uid]->wantsToPay = true;
        sem_post(&paymentServers);
    } else {
        printf("Rider %2d timed out\n", rider->uid);
    }

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

    clock_gettime(CLOCK_REALTIME, t);
    t->tv_sec += seconds;

    return t;
}

void reachDestination(Rider * r) {
    printf("Rider %d found cab %d\n", r->uid, r->cab->uid);

    sleep(r->rideTime); // rider reaching his/her destination

    printf("Rider %d ended his ride\n", r->uid);
}

bool handoverCab(Rider * rider) {
    bool cabWasAssigned = false;
    waitingForCab[rider->uid] = 0;

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
                printf("Handing over cab %d to %d\n", rider->cab->uid, i);
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
            freePoolOnes--;
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
                printf("Handing over cab %d to %d\n", rider->cab->uid, i);
                break;
            }
        }
    }

    return cabWasAssigned;
}
