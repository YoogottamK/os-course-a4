#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

#define PRINT_SORTED 0

typedef struct Args {
    int l,
        r,
        *arr;
} Args;

typedef enum {
    NORMAL,
    PROC,
    THREAD
} Strategy;

/*
 * Returns pointer to addr of shared memory segment
 *  of size 'size' bytes
 */
int * getSharedMemory(size_t size);

/*
 * Sorts an array with 'strategy'
 */
void sort(int * arr, int n, Strategy s);

/*
 * swaps int values present at a and b
 */
void swap(int * a, int * b);

/*
 * The standard partition function for quicksort
 *  searches for x in the range given and then
 *  puts all lesser values to left of x and all
 *  greater values to the right of x
 */
int partition(int * arr, int l, int r);

/*
 * Gives a random number between l and r (inclusine)
 */
int randomInt(int l, int r);

/*
 * copies values from array a to array b
 */
void copy(int * a, int * b, int n);

/*
 * quicksort algo, the normal way
 */
void quickSort(int * arr, int l, int r);

/*
 * quicksort algo, using processes
 */
void quickSortProcess(int * arr, int l, int r);

/*
 * quicksort algo, using threads
 */
void * quickSortThread(void * a);

int main() {
    srand(time(0));

    int n;
    scanf("%d", &n);

    int * arr = getSharedMemory(sizeof(int) * n);
    int * inp = (int *) malloc(sizeof(int) * n);

    for(int i = 0; i < n; i++) {
        scanf("%d", arr + i);
        inp[i] = arr[i];
    }

    sort(arr, n, NORMAL);

    copy(inp, arr, n);

    sort(arr, n, PROC);

    if(n <= 10000) {
        copy(inp, arr, n);
        sort(arr, n, THREAD);
    }

    return 0;
}

int * getSharedMemory(size_t size) {
    key_t key = IPC_PRIVATE;
    int id = shmget(key, size, IPC_CREAT | 0666);

    return (int *)shmat(id, 0, 0);
}

double getRunTime(struct timespec t) {
    double ret = t.tv_sec;
    ret += ((double)t.tv_nsec / 1e9);

    return ret;
}

void swap(int * a, int * b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int randomInt(int l, int r) {
    return rand() % (r - l + 1) + l;
}

int partition(int * arr, int l, int r) {
    int i = randomInt(l, r),
        p = arr[i];

    swap(arr + i, arr + r);

    i = l;
    for(int j = l; j < r; j++) {
        if(arr[j] <= p) {
            swap(arr + i, arr + j);
            i++;
        }
    }
    swap(arr + i, arr + r);

    return i;
}

void sort(int * arr, int n, Strategy s) {
    struct timespec start, end;

    switch(s) {
        case NORMAL:
            printf("Strategy: Normal\n");
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
            quickSort(arr, 0, n - 1);
            break;
        case PROC:
            printf("Strategy: Process\n");
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
            quickSortProcess(arr, 0, n - 1);
            break;
        case THREAD:
            printf("Strategy: Thread\n");
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
            Args args = { .arr = arr, .l = 0, .r = n - 1 };
            quickSortThread(&args);
            break;
        default:
            printf("Unknown strategy %d\n", s);
            exit(1);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    printf("Time taken: %.09lfs\n", getRunTime(end) - getRunTime(start));

    if(PRINT_SORTED) {
        for(int i = 0; i < n; i++)
            printf("%d ", arr[i]);
        printf("\n");
    }
}

void copy(int * a, int * b, int n) {
    for(int i = 0; i < n; i++)
        b[i] = a[i];
}

void quickSort(int * arr, int l, int r) {
    if(l >= r) return;

    if(r - l + 1 <= 5) {
        for(int i = l; i < r; i++) {
            int j = i + 1;

            for(; j <= r; j++)
                if(arr[j] < arr[i] && j <= r)
                    swap(arr + i, arr + j);
        }
        return;
    }

    int p = partition(arr, l, r);

    quickSort(arr, l, p - 1);
    quickSort(arr, p + 1, r);
}

void quickSortProcess(int * arr, int l, int r) {
    if(l >= r) return;

    if(r - l + 1 <= 5) {
        for(int i = l; i < r; i++) {
            int j = i + 1;

            for(; j <= r; j++)
                if(arr[j] < arr[i] && j <= r)
                    swap(arr + i, arr + j);
        }
        return;
    }

    int p = partition(arr, l, r);

    int p1 = fork();
    int p2;

    if(p1 == 0) {
        quickSort(arr, l, p - 1);
        exit(0);
    } else {
        p2 = fork();
        if(p2 == 0) {
            quickSort(arr, p + 1, r);
            exit(0);
        } else {
            waitpid(p1, 0, 0);
            waitpid(p2, 0, 0);
        }
    }
}

void * quickSortThread(void * args) {
    Args * a = (Args*) args,
         a1,
         a2;

    if(a->l >= a->r) return 0;

    if(a->r - a->l + 1 <= 5) {
        for(int i = a->l; i < a->r; i++) {
            int j = i + 1;

            for(; j <= a->r; j++)
                if(a->arr[j] < a->arr[i] && j <= a->r)
                    swap(a->arr + i, a->arr + j);
        }
        return 0;
    }

    int p = partition(a->arr, a->l, a->r);

    a1.arr = a2.arr = a->arr;

    a1.l = a->l;
    a1.r = p - 1;

    a2.l = p + 1;
    a2.r = a->r;

    pthread_t t1, t2;
    pthread_create(&t1, 0, quickSortThread, &a1);
    pthread_create(&t2, 0, quickSortThread, &a2);

    pthread_join(t1, 0);
    pthread_join(t2, 0);

    return 0;
}
