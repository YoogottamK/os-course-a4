#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "main.h"
#include "robot.h"
#include "table.h"
#include "student.h"

void init() {
    srand(time(0));

    tables = (Table **) malloc(sizeof(Table *) * N);
    robots = (Robot **) malloc(sizeof(Robot *) * M);
    students = (Student **) malloc(sizeof(Student *) * K);

    robot_t = (pthread_t *) malloc(sizeof(pthread_t) * M);
    table_t = (pthread_t *) malloc(sizeof(pthread_t) * N);
    student_t = (pthread_t *) malloc(sizeof(pthread_t) * K);

    for(int i = 0; i < M; i++)
        initRobot(i);

    for(int i = 0; i < N; i++)
        initTable(i);

    for(int i = 0; i < K; i++)
        initStudent(i);
}

int main() {
    scanf("%d %d %d", &N, &M, &K);

    init();

    for(int i = 0; i < K; i++)
        pthread_join(student_t[i], 0);

    // check if students are still eating
    for(int i = 0; i < N; i++) {
        char * students = (char *) malloc(1024);
        char * num = (char *) malloc(10);
        students[0] = 0;
        num[0] = 0;

        for(int j = 0; j < 10; j++) {
            if(tables[i]->eating[j] >= 0) {
                sprintf(num, "%d ", tables[i]->eating[j]);
                strcat(students, num);
                tables[i]->eating[j] = -1;
            }
        }

        if(students[0])
            printf("Serving table %d has served students %s\n", i, students);

        pthread_cancel(table_t[i]);
    }

    return 0;
}

int randRange(int l, int r) {
    if(l > r)
        return l;

    return (rand() % (r - l + 1)) + l;
}
