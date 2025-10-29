%%writefile parallel_sort_processes.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h> // used for shared memory for IPC
#include <sys/wait.h>
#include <time.h>

// Compare function for qsort
int cmp(const void *a,const void *b){
    int x=*(int*)a, y=*(int*)b;
    return (x>y)-(x<y);
}

int main(int argc,char **argv){
    if(argc<3){
        printf("Usage: %s <size> <workers>\n",argv[0]);
        return 1;
    }

    int n=atoi(argv[1]), workers=atoi(argv[2]);
    // create shared memory for array so all processes can see same array
    int *arr=mmap(NULL,n*sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    srand(42);
    for(int i=0;i<n;i++) arr[i]=rand();

    struct timespec t0,t1;
    clock_gettime(CLOCK_MONOTONIC,&t0); // start timestamp

    // Fork workers to sort their chunks - map phase
    for(int i=0;i<workers;i++){
        pid_t pid=fork();
        if(pid==0){
            int chunk=(n+workers-1)/workers;
            int l=i*chunk, r=l+chunk-1; if(r>=n) r=n-1; if(l>=n) _exit(0);
            qsort(arr+l,r-l+1,sizeof(int),cmp);
            _exit(0);
        }
    }

    // Wait for all children to finish sorting
    for(int i=0;i<workers;i++) wait(NULL);

    clock_gettime(CLOCK_MONOTONIC,&t1); // end timestamp

    // Merge sorted chunks - reduce phase
    int *tmp = malloc(n*sizeof(int));
    int active=workers;
    int chunk=(n+workers-1)/workers;

    while(active>1){
        int pairs=active/2;
        for(int p=0;p<pairs;p++){
            int l1=p*2*chunk, r1=l1+chunk-1; if(r1>=n) r1=n-1; // left and right index of first chunk
            int l2=l1+chunk, r2=l2+chunk-1; if(r2>=n) r2=n-1;  // left and right index of second chunk

            // 2 pointer merge to sort the two chunks like merge sort
            int i=l1,j=l2,k=0;
            while(i<=r1 && j<=r2) tmp[k++] = (arr[i]<=arr[j]?arr[i++]:arr[j++]);
            while(i<=r1) tmp[k++] = arr[i++];
            while(j<=r2) tmp[k++] = arr[j++];
            for(i=0;i<k;i++) arr[l1+i]=tmp[i];
        }
        // after each pass, the number of active processes is halved
        active=(active+1)/2;
        // chunk size is doubled
        chunk*=2;
    }

    // get evaluation metrics
    double time_ms=(t1.tv_sec-t0.tv_sec)*1000.0 + (t1.tv_nsec-t0.tv_nsec)/1000000.0;
    size_t mem_bytes = n*sizeof(int) + n*sizeof(int);
    double mem_MB = mem_bytes/(1024.0*1024.0);

    printf("workers: %d\n",workers);
    printf("time: %.3f ms\n",time_ms);
    printf("memory usage: %.6f MB\n",mem_MB);

    free(tmp);
    munmap(arr,n*sizeof(int));
    return 0;
}

