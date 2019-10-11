#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "main.h"
#include "student.h"

const int MAX_ARRIVAL_TIME = 10;

Table * wait_for_slot(Student * s) {
    bool gotSlot = false;
    Table * t;

    while(!gotSlot) {
        for(int i = 0; i < N; i++) {
            pthread_mutex_lock(&tables[i]->mutex);

            if(tables[i]->slots > 0) {
                gotSlot = true;

                printf("Student %d assigned to a slot on the serving table "
                        "%d and waiting to be served\n",
                        s->uid, i);

                tables[i]->slots--;
                t = tables[i];

                for(int j = 0; j < 10; j++)
                    if(tables[i]->eating[j] < 0)
                        tables[i]->eating[j] = s->uid;

                pthread_mutex_unlock(&tables[i]->mutex);
                break;
            }

            pthread_mutex_unlock(&tables[i]->mutex);
        }
    }

    return t;
}

void * studentRunner(void * a) {
    Student * s = (Student *)a;

    sleep(s->arrivalTime);
    printf("Student %d has arrived\n", s->uid);
    printf("Student %d is waiting to be allocated "
            "a slot on the serving table\n", s->uid);

    Table * t = wait_for_slot(s);

    return 0;
}

void initStudent(int i) {
    Student * s = (Student *) malloc(sizeof(Student));

    s->uid = i;
    s->arrivalTime = rand() % MAX_ARRIVAL_TIME;

    students[i] = s;

    pthread_create(&student_t[i], 0, studentRunner, s);
}
