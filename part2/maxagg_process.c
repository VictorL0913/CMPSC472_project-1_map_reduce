%%writefile maxagg_process.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>


int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s <size> <workers>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int workers = atoi(argv[2]);

    // array of random numbers (local memory)
    int *arr = malloc(n * sizeof(int));
    srand(42);
    for(int i=0;i<n;i++) arr[i] = rand();

    // Shared memory for global max
    int *global_max = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE,
                           MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *global_max = arr[0];

    // Shared mutex for synchronization across process
    pthread_mutex_t *lock = mmap(NULL, sizeof(pthread_mutex_t),
                                 PROT_READ|PROT_WRITE,
                                 MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(lock, &attr);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);  // start timestamp

    // Fork workers to compute local max and update global max
    for(int i=0;i<workers;i++){
        pid_t pid = fork();
        if(pid == 0){
            // Compute chunk boundaries
            int chunk = (n + workers - 1) / workers;
            int l = i * chunk;
            int r = l + chunk - 1;
            if(r >= n) r = n - 1;
            if(l >= n) _exit(0);

            // Local max computation
            int local_max = arr[l];
            for(int j = l + 1; j <= r; j++)
                if(arr[j] > local_max) local_max = arr[j];

            // Update global max with locking to ensure atomicity
            pthread_mutex_lock(lock);
            if(local_max > *global_max) *global_max = local_max;
            pthread_mutex_unlock(lock);

            _exit(0);
        }
    }

    // Wait for all children
    for(int i=0;i<workers;i++) wait(NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);  // end timestamp

    // Get evaluation metrics
    double time_ms = (t1.tv_sec - t0.tv_sec)*1000.0 +
                     (t1.tv_nsec - t0.tv_nsec)/1000000.0;
    size_t mem_bytes = n*sizeof(int) + sizeof(int);
    double mem_MB = mem_bytes / (1024.0*1024.0);

    // output
    printf("workers: %d\n", workers);
    printf("array size: %d\n", n);
    printf("global max value: %d\n", *global_max);
    printf("time: %.3f ms\n", time_ms);
    printf("memory usage: %.6f MB\n", mem_MB);

    free(arr);
    pthread_mutex_destroy(lock);
    munmap(global_max, sizeof(int));
    munmap(lock, sizeof(pthread_mutex_t));
    return 0;
}

