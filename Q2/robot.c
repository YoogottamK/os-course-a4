#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "robot.h"

void biryani_ready(Robot * r) {
    while(r->vessels->count > 0) {
        for(int i = 0; i < N; i++) {
            pthread_mutex_lock(&tables[i]->mutex);

            if(!tables[i]->container) {
                Container * c = (Container *) malloc(sizeof(Container));
                c->capacity = r->vessels->capacity;
                r->vessels->count--;

                printf("Serving container of table %d is refilled"
                        " by Robot Chef %d; Table %d resuming serving now\n",
                        i, r->uid, i);

                tables[i]->container = c;
            }

            pthread_mutex_unlock(&tables[i]->mutex);
        }
    }
}

void * robotRunner(void * a) {
    Robot * r = (Robot *)a;

    while(1) {
        Vessels * v = (Vessels *) malloc(sizeof(Vessels));
        v->count = randRange(1, 10);
        v->capacity = randRange(25, 50);

        r->vessels = v;

        printf("Robot Chef %d is preparing %d vessels of biryani\n",
                r->uid, v->count);
        sleep(randRange(2, 5)); // preparing r vessels
        printf("Robot Chef %d has prepared %d vessels of biryani. "
                "Waiting for all the vessels to be emptied to resume cooking\n",
                r->uid, v->count);

        biryani_ready((Robot *)a);
    }

    return 0;
}

void initRobot(int i) {
    Robot * r = (Robot *) malloc(sizeof(Robot));

    r->uid = i;
    r->table = 0;
    r->vessels = 0;
    pthread_mutex_init(&r->mutex, 0);

    robots[i] = r;

    pthread_create(&robot_t[i], 0, robotRunner, r);
}
