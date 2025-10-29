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
    
#### Multithreading vc Multiprocessing

Although both methods will allw parallelism, they differ:
  1. Multithreading
     - uses multiple threads within the same process to access shared memory for fast communication, but will need           synchronization to acoid race conditions
  2. Multiprocessing
     - will use separate processes, which do not share memory (by default), to communicate using IPC mechanisms like shared memory or pipes. This will allow stronger memory safety, but will cause larger overhead.
       
#### Project tasks





### Structure of Code

### Description of Tools and Methods

### Performance Evaluation

### Conclusion




