#include <stdlib.h>

#include "main.h"
#include "cab.h"

void initCab(int uid) {
    Cab * c = (Cab *) getSharedMemory(sizeof(Cab));

    c->state = waitState;
    c->uid = uid;
    c->r1 = c->r2 = 0;

    allCabs[uid] = c;
}
