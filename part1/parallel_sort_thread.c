
%%writefile parallel_sort_thread.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int *arr;
int n, workers;

// Compare function for qsort
int cmp(const void *a, const void *b){
    int x = *(int*)a, y = *(int*)b;
    return (x>y) - (x<y);
}

// Merge two sorted subarrays into one - reduce phase
void merge(int *a,int l1,int r1,int l2,int r2,int *tmp){
    int i=l1,j=l2,k=0;
    while(i<=r1 && j<=r2) tmp[k++] = (a[i]<=a[j]?a[i++]:a[j++]);
    while(i<=r1) tmp[k++] = a[i++];
    while(j<=r2) tmp[k++] = a[j++];
    for(i=0;i<k;i++) a[l1+i]=tmp[i];
}

// Function run by each thread to sort its chunk - map phase
void *thread_sort(void *arg){
    long id=(long)arg;
    int chunk=(n+workers-1)/workers;
    int l=id*chunk;
    if(l>=n) return NULL;
    int r=l+chunk-1; if(r>=n) r=n-1;
    qsort(arr+l,r-l+1,sizeof(int),cmp);
    return NULL;
}

int main(int argc,char **argv){
    if(argc<3){
        printf("Usage: %s <size> <workers>\n",argv[0]);
        return 1;
    }

    n=atoi(argv[1]);
    workers=atoi(argv[2]);
    arr=malloc(n*sizeof(int));
    srand(42);
    for(int i=0;i<n;i++) arr[i]=rand();

    pthread_t th[workers];
    struct timespec t0,t1;
    clock_gettime(CLOCK_MONOTONIC,&t0);

    for(long i=0;i<workers;i++) pthread_create(&th[i],NULL,thread_sort,(void*)i);
    for(int i=0;i<workers;i++) pthread_join(th[i],NULL);

    clock_gettime(CLOCK_MONOTONIC,&t1);

    int *tmp=malloc(n*sizeof(int)); // temp buffer for merging
    int active=workers;
    int chunk=(n+workers-1)/workers;

    while(active>1){
        int pairs=active/2;
        for(int p=0;p<pairs;p++){
            int l1=p*2*chunk, r1=l1+chunk-1; if(r1>=n) r1=n-1;
            int l2=l1+chunk, r2=l2+chunk-1; if(r2>=n) r2=n-1;
            merge(arr,l1,r1,l2,r2,tmp); // merge adjacent chunks
        }
        active=(active+1)/2; // halve the number of active threads
        chunk*=2; // chunk size doubles after each iteration
    }

    // get evaluation metrics
    double time_ms=(t1.tv_sec-t0.tv_sec)*1000.0 + (t1.tv_nsec-t0.tv_nsec)/1000000.0;
    size_t mem_bytes = n*sizeof(int) + n*sizeof(int);
    double mem_MB = mem_bytes / (1024.0*1024.0);

    printf("workers: %d\n",workers);
    printf("time: %.3f ms\n",time_ms);
    printf("memory usage: %.6f MB\n",mem_MB);

    free(tmp); free(arr);
    return 0;
}
