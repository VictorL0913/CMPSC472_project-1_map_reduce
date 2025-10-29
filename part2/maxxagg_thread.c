%%writefile maxagg_thread.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int *arr;             // main array
int n, workers;       // array size and number of threads
int global_max;       // shared max value
pthread_mutex_t lock; // mutex for synchronization

// Function for each thread to compute local max and update global max 
void *thread_max(void *arg){
    long id = (long)arg;
    int chunk = (n + workers - 1) / workers;
    int l = id * chunk;
    if(l >= n) return NULL;
    int r = l + chunk - 1; if(r >= n) r = n - 1;
    int local_max = arr[l];
    for(int i = l+1; i <= r; i++) if(arr[i] > local_max) local_max = arr[i];

    pthread_mutex_lock(&lock);           // lock before updating global max
    if(local_max > global_max) global_max = local_max;
    pthread_mutex_unlock(&lock);         // unlock
    return NULL;
}

int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s <size> <workers>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);
    workers = atoi(argv[2]);
    arr = malloc(n * sizeof(int));
    srand(42);
    for(int i=0;i<n;i++) arr[i]=rand();

    global_max = arr[0];
    pthread_mutex_init(&lock, NULL);

    pthread_t th[workers];
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    // create threads - map phase
    for(long i=0;i<workers;i++) pthread_create(&th[i], NULL, thread_max, (void*)i);
    for(int i=0;i<workers;i++) pthread_join(th[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double time_ms = (t1.tv_sec - t0.tv_sec)*1000.0 + (t1.tv_nsec - t0.tv_nsec)/1000000.0;
    size_t mem_bytes = n*sizeof(int) + sizeof(int);
    double mem_MB = mem_bytes / (1024.0*1024.0);

    // get evaluation metrics - reduce phase 
    printf("workers: %d\n", workers);
    printf("max value: %d\n", global_max);
    printf("time: %.3f ms\n", time_ms);
    printf("memory usage: %.6f MB\n", mem_MB);

    free(arr);
    pthread_mutex_destroy(&lock);
    return 0;
}

