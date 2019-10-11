#include <pthread.h>

#ifndef __MAIN_H
#define __MAIN_H

typedef struct Robot {
    int uid;

    struct Table * table;
    struct Vessels * vessels;

    pthread_mutex_t mutex;
} Robot;

typedef struct Table {
    int uid,
        slots;

    struct Container * container;
    struct Robot * robot;

    int eating[10];

    pthread_mutex_t mutex;
} Table;

typedef struct Student {
    int uid,
        arrivalTime;
} Student;

typedef struct Vessels {
    int count,
        capacity;
} Vessels;

typedef struct Container {
    int capacity;
} Container;

Robot ** robots;

Table ** tables;

Student ** students;

int M, // number of robots
    N, // number of tables
    K; // number of students

int randRange(int l, int r);

pthread_t * table_t,
          * student_t,
          * robot_t;

#endif // __MAIN_H
