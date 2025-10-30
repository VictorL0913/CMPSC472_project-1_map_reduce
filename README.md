# CMPSC472 Project 1 MapReduce

### Project Overview

This project experiments with parallel computation using the MapReduce programming model on a single machine by implementing multithreading and multiprocessing approaches. The goal is to learn how operating systems allow parallelism and how different implementations affect performance when managing shared data or synchronization. 

#### MapReduce

- MapReduce is a programming model that is designed to process and generate large datasets across distributed clusters of machines to simplify parallel computation by handling complex tasks like splitting data, fault tolerance, and coordinating work so the programmer does not need to.
- While this project is inspired by the programming model and runtime system, it will be run on a single machine using:
  1. Multithreading and multiprocessing
  2. Local IPC mechanisms like shared memory
  3. Single reducer process or thread to aggregate results
- The implementation of the MapReduce model will divide the computation into two phases:
  * Map phase - the input is split into chunks and processed independently in parallel using merge sort or quick sort
  * Reduce phase - the partial results are combines to produce a final result
    
#### Multithreading vs. Multiprocessing

Although both methods will allw parallelism, they differ:
  1. Multithreading
     - uses multiple threads within the same process to access shared memory for fast communication, but will need           synchronization to acoid race conditions
  2. Multiprocessing
     - will use separate processes, which do not share memory (by default), to communicate using IPC mechanisms like shared memory or pipes. This will allow stronger memory safety, but will cause larger overhead.
       
#### Assignment Tasks

**Part 1: Parallel Sorting (MapReduce Style)**
- Input: Large array of integers (two lengths: 32, 131,072)
- Map Phase: Divide the array into chunks and sort each chunk in parallel using Merge Sort or Quick Sort.
- Reduce Phase: Merge sorted chunks into a final sorted array.
- Implementation requirements
- Develop a program for multithreading and multiprocessing.
- Use IPC (e.g., message passing or shared memory) to pass sorted chunks to the reducer.
- Measure execution time and memory usage for 1, 2, 4, and 8 workers for map phase
 
**Part 2: Max-Value Aggregation with Constrained Shared Memory**
- Task: Compute the global maximum value.
- Map Phase: Each worker computes the local maximum of its chunk. Workers attempt to update a shared memory buffer that holds only one value — the current global maximum.
- Reduce Phase: A single reducer reads the final value from the shared buffer.
- Synchronization is required to prevent race conditions when multiple workers attempt to update the buffer.
- The shared memory region is limited to one integer.
- Workers must read the current value, compare it with their local max and write the new value only if it's larger.
- Measure performance impact of synchronization for 1, 2, 4, and 8 workers

**How To Run**
- To run the project - use the Google Colab link to run the cells: https://colab.research.google.com/drive/1Tn-eD0QTtMYzaM8Xp269MGuLHC4QWKY_#scrollTo=eNOy6whF2fDB
- Another way is to copy the .c files in part1 and part2 folder to own notebook any run using shell commands like:
  ```bash
  !gcc parallel_sort_thread.c -pthread -o p1_threads  // replace based on file 
  !./p1_threads 32 1
  !./p1_threads 32 2
  !./p1_threads 32 4
  !./p1_threads 32 8
  !./p1_threads 131072 1
  !./p1_threads 131072 2
  !./p1_threads 131072 4
  !./p1_threads 131072 8
  ```

  or

  ```bash
  !gcc parallel_sort_processes.c -o p1_process
  !./p1_process 32 1
  !./p1_process 32 2
  !./p1_process 32 4
  !./p1_process 32 8
  !./p1_process 131072 1
  !./p1_process 131072 2
  !./p1_process 131072 4
  !./p1_process 131072 8
  ```

### Structure of Code

#### Part 1: Parallel Sort 

![alt text](https://github.com/VictorL0913/CMPSC472_project-1_map_reduce/blob/main/Diagrams/parallel_sort_threads.png) 

In the multithread version, a single process creates multiple threads using pthread_create() and each thread share the same space and memory so no IPC mechanism is required. The threads will directly access and modify the global raay during the map phase when they sort the chunks that they are assigned, After the threads are complete teh main thread performs the reduce phase which iteratively merge the sorted chunks within the shared memory.
  
![alt text](https://github.com/VictorL0913/CMPSC472_project-1_map_reduce/blob/main/Diagrams/parallel_sort_process_diagram.png)

In the multiprocess version, the parent process creates child processes using fork(), and since each process has their own memory, some kind of IPC is required. In this project, the array is allocated in a shared memory region using mmap(), which allows all processes to read and write the same data. Each process will sort their assigned chunks in the Map phase, and after all the processes exit, the parent process performs the reduce phase by merging the sorted chunks from the shared memory. 


#### Part 2: Max-Value Aggregation

![alt text](https://github.com/VictorL0913/CMPSC472_project-1_map_reduce/blob/main/Diagrams/maxagg_thread_diagram.png) 

In the multithread version, the main array is divided into chunks and each thread compute the local max for the chunks that they are assigned. In the map phase, the threads will be concurrently work on portions of the data. After calculating the local max, each thread will try to update the global max which is the reduce phase. A mutex is used to ensure that only 1 thread can modify the global max at a time to prevent cases of race conditions.

![alt text](https://github.com/VictorL0913/CMPSC472_project-1_map_reduce/blob/main/Diagrams/maxagg_pro_diagram.png) 

In the multiprocess version, each worker is created using fork(), and each process computes the local max for each of the assigned chunks (map phase). Since processes have separate memory spaces, the global max is stored in shared memory using mmap. Synchronization is implemented by using a process shared mutex so that updates to the global max occur atomically during the reduce phase. 

### Description of Tools and Methods

#### Part 1

##### Multithread implementation

In the multithread implementation, pthread.h, stdlib.h, and time.h libraries were mainly used for thread creation, memory management, qsort, and timestamp. A single main process will create worker threads specified at runtime using pthread_create(), and each thread will be assigned a chunk of the array in parallel -- the map phase. Threads share the same memory space so no inter-process communication is necessary since all the data will be read/write using the shared array. Syncchronization is used to make sure that the reduce phase does not start before all the threads have sorted their assigned chunks using pthread_join(). Since each thread will only access their assigned chunks then there is no need for other synchronization mechanisms like mutex. Performance is measured using clock_gettime(CLOCK_MONOTONIC) to get the execution time for both phases and memory usage is estimated based on the array plus the temp merge buffer. This approach allows the measurement to evaluate the parallelism without the OS scheduling delays and other overheads that are not necessarily part of the algorithm.

##### Multiprocess implementation

The multiprocess implementation uses fork() to create child processes and sys/mman.h to allocate shared memory via mmap() for IPC. Each child process sorts their assigned chunk of the array in parallel in the map phase, and the parent process will perform the reduce phase by merging the sorted chunked from the shared memory. In this implementation, synchronization is critical to make sure that parent process will wait until all child processes are finished before starting the reduce phase. Performance is measured using the same approach as the multithreading implemenation to ensure fairness when comparing both implementations. 

#### Part 2

##### Multithread implementation

In the multithread implementation, it uses the POSIX pthread library in C. Threads are manually created depending on the worker threads that is specified at runtime. Each thread is assigned a chunk of the array and performs a local max computation with its chunk. Since all threads are within the same process, no explicit IPC is required, so threads work together using shared memory. A pthread_mutex_t lock is implemented to protext the global max value so that there are no race conditions in the updating phase. Performance is measured using clock_gettime(CLOCK_MONOTONIC) with time starting before thread creation and after the pthread_join() is called so that the code is measuring the parallel execution cost. Memory usage is calculated deterministically using n*sizeof(int) for the array plus sizeof(int) for the shared max since information from OS would include readings for usage that is not solely the algorithm like process metadata and page alignment. 

##### Multiprocess implementation

The multiprocess version implementation uses fork() to create worker processes, and since processes do not share memory by default, the global max is stored in a shared memory region created using mmap (MAP_SHARE | MAP_ANNONYMOUS). A pthread_mutex_t is setup so that all forked processes can the lock. This allows only one process to access the global max during the updating phase. When the child processes finish computing the local max, each child process will lock the mutex, check if its local value is higher than the global value, then updates and release the lock. This lets multiple processes to corrdinate updates so that there are no race conditions. For the performance, the multiprocess implementation uses the same approach as the threading approach so that it is a controlled comparison between both implementations. 


### Performance Evaluation

![alt text](https://github.com/VictorL0913/CMPSC472_project-1_map_reduce/blob/main/Screenshot/parallel_sort_process.png) 
![alt text](https://github.com/VictorL0913/CMPSC472_project-1_map_reduce/blob/main/Screenshot/parallel_sort_process.png)

The correctness for both implementations was verified using a small array of size 32. In the multithread implemenation the execution times decreased as the number of workers increased from 1 to 8 that ranged from 27.215ms to 16.797ms when the input size was 131,072. With the same input, the multiprocess version showed decreases initially, but increased when there was 8 processes (4 works timed at 17.168ms with 8 workers timed at 19.217ms). This reflects the additional overhead that comes with the multiprocess implementation with shared memory and process management. Threads are faster since they share the same memory and require minimal synchronization mechanism. Multiprocess need mmap() for shared memory and wait() for synchronization, which adds overhead. With more processes created (8), the time reflects the additional overhead that is caused by mmap() and wait(). Overall, threads scale better as worker count is increased since they share the same memory space and require less synchronization which is why threads have less overhead. Multiprocess causes extra overhead due to IPC and process management which caused the inefficiencies in performance as worker counts increased.

![alt text](https://github.com/VictorL0913/CMPSC472_project-1_map_reduce/blob/main/Screenshot/maxagg_process.png) 
![alt text](https://github.com/VictorL0913/CMPSC472_project-1_map_reduce/blob/main/Screenshot/maxagg_thread.png) 

The correctness for both implementations was verified using a small array of size 32. Testing with [1,2,4,8] worker counts in both the multithreading and multiprocessing implementations showed the same global max which implies that the mutex based synchronization prevented any race conditions to occur. In the multithread implementation, execution time ranged from 0.671ms to 0.760ms, and the multiprocess version ranged from 0.916ms to 2.112ms when the input array was length 131,072. The time comparison shows that multithreading outperforms multiprocessing for larger arrays which is reasonable since threads share the same memory space so threads have less overhead. Multiprocess require additional overhead for fork() operation, memory mapping, and interprocess synchronizations. Synchronization was implemented in both versions: threads used pthread_mutex_t and process used PTHREAD_PROCESS_SHARED. Multithreading offers better performance with less overhead and multiprocess allows for better memory security, but will require additional overhead for IPC and synchronization costs.


### Conclusion

This project experiments with the MapReduce programming model to implement MapReduce style parallel computing using multithreading and multiprocessing on a single machine. Multithreading implementations showed faster execution times since they share the same memory space and require less overhead. Multiprocess approaches has more security with memory, but require additional overhead as the implementation needs to use extra process management and synchronization steps. There are some limitations to the project like simplified model that only runs on a single machine, approximate memory measurements, and simplified implementation. For future implementations, improvements could include testing other synchronization mechanisms to test efficiency or implement a more detailed performance metric to test the approaches to get a better understanding on the tradeoffs in each approach. 




