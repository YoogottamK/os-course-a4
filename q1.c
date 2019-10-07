#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <time.h>
#include <stdlib.h>

typedef struct args {
    int l,
        r,
        *arr;
} args;

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
 * quicksort algo, the normal way
 */
void quickSort(int * arr, int l, int r);

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

int main() {
    srand(time(0));

    int n;
    scanf("%d", &n);

    int * arr = getSharedMemory(sizeof(int) * n);

    for(int i = 0; i < n; i++)
        scanf("%d", arr + i);

    sort(arr, n, NORMAL);

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
            break;
        case THREAD:
            printf("Strategy: Thread\n");
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
            break;
        default:
            printf("Unknown strategy %d\n", s);
            exit(1);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    printf("Time taken: %.09lfs\n", getRunTime(end) - getRunTime(start));
    for(int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

void quickSort(int * arr, int l, int r) {
    if(l >= r) return;

    int p = partition(arr, l, r);

    quickSort(arr, l, p - 1);
    quickSort(arr, p + 1, r);
}
