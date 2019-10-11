#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "table.h"

int min(int a, int b) {
    return a < b ? a : b;
}

void ready_to_serve_table(Table * t, int slots) {
    printf("Serving table %d is ready to serve with %d slots\n", t->uid, slots);
    pthread_mutex_lock(&t->mutex);

    t->slots = slots;
    t->container->capacity -= slots;

    pthread_mutex_unlock(&t->mutex);

    while(t->slots);

    char * students = (char *) malloc(1024);
    char * num = (char *) malloc(10);
    students[0] = 0;

    for(int i = 0; i < 10; i++) {
        if(t->eating[i] > 0) {
            snprintf(num, 10, "%d ", t->eating[i]);
            strcat(students, num);
            t->eating[i] = -1;
        }
    }

    printf("Serving table %d has served students %s\n", t->uid, students);
}

void * tableRunner(void * a) {
    Table * t = (Table *)a;

    while(1) {
        while(!t->container);

        printf("Serving table %d entering serving phase\n", t->uid);

        while(t->container->capacity > 0)
            ready_to_serve_table(t, randRange(1, min(10, t->container->capacity)));

        printf("Serving table %d is empty, waiting to be refilled\n", t->uid);
    }

    return 0;
}

void initTable(int i) {
    Table * t = (Table *) malloc(sizeof(Table));

    t->uid = i;
    t->robot = 0;
    t->slots = 0;
    t->container = 0;
    pthread_mutex_init(&t->mutex, 0);

    for(int i = 0; i < 10; i++)
        t->eating[i] = -1;

    tables[i] = t;

    pthread_create(&table_t[i], 0, tableRunner, t);
}
