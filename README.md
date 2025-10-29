# CMPSC472_project-1_map_reduce

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

Part 1: Parallel Sorting (MapReduce Style)
- Input: Large array of integers (two lengths: 32, 131,072)
- Map Phase: Divide the array into chunks and sort each chunk in parallel using Merge Sort or Quick Sort.
- Reduce Phase: Merge sorted chunks into a final sorted array.
- Implementation requirements
- Develop a program for multithreading and multiprocessing.
- Use IPC (e.g., message passing or shared memory) to pass sorted chunks to the reducer.
- Measure execution time and memory usage for 1, 2, 4, and 8 workers for map phase
 
Part 2: Max-Value Aggregation with Constrained Shared Memory
- Task: Compute the global maximum value.
- Map Phase: Each worker computes the local maximum of its chunk. Workers attempt to update a shared memory buffer that holds only one value — the current global maximum.
- Reduce Phase: A single reducer reads the final value from the shared buffer.
- Synchronization is required to prevent race conditions when multiple workers attempt to update the buffer.
- The shared memory region is limited to one integer.
- Workers must read the current value, compare it with their local max and write the new value only if it's larger.
- Measure performance impact of synchronization for 1, 2, 4, and 8 workers

### Structure of Code

#### Part 1: Parallel Sort 

1. Multithread
    1. 
2. Multiprocess
   
#### Part 2: Max-Value Aggregation

1. Multithread

2. Multiprocess

### Description of Tools and Methods

### Performance Evaluation

### Conclusion




