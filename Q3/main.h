#include <stdbool.h>
#include <assert.h>

#include "semaphore.h"

#ifndef __MAIN_H
#define __MAIN_H

/*
 * Ideally, these should have been in their
 *  own header files, but they were causing
 *  a problem [cyclic dependency], so, I ended
 *  up putting all of them here
 */
typedef enum CabState {
    waitState,
    onRidePremier,
    onRidePoolOne,
    onRidePoolFull
} CabState;

typedef enum CabType {
    PREMIER = 1,
    POOL = 2
} CabType;

typedef struct Cab {
    int uid;
    struct Rider * r1, * r2;
    CabState state;
} Cab;

typedef struct Rider {
    int uid,
        waitTime,
        rideTime,
        arrivalTime;

    bool wantsToPay;

    CabType cabType;
    Cab * cab;
} Rider;

int N, // number of cabs
    M, // number of riders
    K, // number of payment servers
    freePoolOnes, // number of cabs in state onRidePoolOne
    freeCabs;     // number of cabs in state waitState

sem_t paymentServers; // semaphore for payment servers

pthread_mutex_t accessCabs,
                paymentMutex;

pthread_t ** servers_t, // will contain thread ids for all the servers
      ** riders_t;  // will contain thread ids for all the riders

/*
 * Contains conditions for all the riders
 *  to be used for conditional wait
 *
 * pthread_cond_timedwait
 */
pthread_cond_t * condWait;

/*
 * The waiting array.
 *  Riders will indicate if they are waiting
 *  for some cab here.
 */
CabType * waitingForCab;

/*
 * Cab array: contains info on all cabs
 */
Cab ** allCabs;

/*
 * Rider array: contains info on all riders
 */
Rider ** allRiders;


void * getSharedMemory(size_t size);

#endif // __MAIN_H
