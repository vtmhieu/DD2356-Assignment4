# Exercise 2: MPI Matrix Parallelization

## Parallelization Strategy

The matrix row-summation program is parallelized using MPI collective communications through the following steps:

1. **Data Distribution (`MPI_Scatter`):** The root process (rank 0) initializes the $N \times N$ matrix and uses `MPI_Scatter` to evenly distribute blocks of contiguous rows to all participating MPI processes. Each process receives a sub-matrix of size `(N / size) x N`.
2. **Local Computation:** Each process independently calculates the sum of each row in its assigned sub-matrix, storing the results in a local array (`local_row_sums`). It also computes a `local_total_sum` for its block.
3. **Data Collection (`MPI_Gather`):** Once local computations are complete, `MPI_Gather` is utilized to collect all the `local_row_sums` arrays from every process back into the full `row_sums` array on the root process.
4. **Global Reduction (`MPI_Reduce`):** The `local_total_sum` computed by each process is aggregated using `MPI_Reduce` with the `MPI_SUM` operation. The root process receives the final `global_total_sum` of all matrix elements.
5. **Synchronization and Benchmarking:** `MPI_Barrier` is called right before and after the computation phase to synchronize all processes. `MPI_Wtime()` is used to measure the execution time of the parallelized section accurately.

## Validation

To validate the correctness of the parallel implementation, we compared the outputs of both the serial and parallel versions. Both programs compute the sum of each row and output the results to a text file. A direct comparison of the generated output files shows 0 differences, confirming that the parallel MPI implementation produces exactly the same numerical results as the original serial version.

### Visualizations

The following plots visualize the row sums computed by the serial and parallel versions, confirming their identical output:

**Serial Implementation:**
![Serial Row Sum Computation](./plot_serial.png)

**Parallel MPI Implementation:**
![Parallel Row Sum Computation](./plot_parallel.png)

### Evaluate performance scaling

#### On school cluster

```bash
jovyan@jupyter-hieuvtm:~/DD2356-Assignment4/ex2$ bash run_weak_scaling.sh
Running Weak Scaling Experiment...
Processes | Matrix Size (N) | Runtime (s)
-----------------------------------------
        1 |            1000 | 0.001574
        2 |            1414 | 0.003270
        4 |            2000 | 0.008479
        8 |            2832 | 0.016070
       16 |            4000 | 0.006481
```

### On Dardel

```bash

```