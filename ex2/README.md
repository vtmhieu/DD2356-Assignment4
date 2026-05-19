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

## Performance Scaling Evaluation

### 1. School Cluster Analysis

**Weak Scaling Plot:**
![Weak Scaling Performance - School Cluster](./weak_scaling_school.png)

**Commands used:**
To achieve weak scaling, the matrix size $N$ was dynamically passed to the compiler to ensure constant work per process ($N = 1000 \times \sqrt{P}$). The code was compiled and executed using the following bash commands for each process count $P$:
```bash
mpicc -O3 hw4-mpi-coll-TBC-new.c -o parallel_scaling -DN=$N
mpirun -np $P ./parallel_scaling
```

```bash
Processes | Matrix Size (N) | Runtime (s)
-----------------------------------------
        1 |            1000 | 0.001574
        2 |            1414 | 0.003270
        4 |            2000 | 0.008479
        8 |            2832 | 0.016070
       16 |            4000 | 0.006481
```

**Analysis:**
In an ideal weak scaling scenario (constant work per process), the runtime should remain completely flat. Instead, the runtime increases significantly from 1 to 8 processes (0.0015s to 0.016s). This poor scaling efficiency occurs because the mathematical computation is exceptionally fast, causing the execution time to be completely dominated by the overhead of MPI collective communications (`MPI_Scatter`, `MPI_Gather`, `MPI_Reduce`).
Interestingly, at 16 processes, the execution time drops to 0.0064s. This is likely due to beneficial cache effects; as the total matrix size reaches $4000 \times 4000$, the 16 processes split the data into much smaller, cache-friendly row chunks (250 rows each), yielding a computational speedup that partially offsets the communication overhead.

### 2. Dardel Cluster Analysis

**Weak Scaling Plot:**
![Weak Scaling Performance - Dardel](./weak_scaling_dardel.png)

**Commands used:**
The job was submitted using a Slurm batch script (`sbatch dardel_weak_scaling.sh`) configured with `--ntasks-per-node=4`. Inside the script, the code was compiled with the standard Cray compiler wrapper and executed using `srun`:
```bash
cc -O3 hw4-mpi-coll-TBC-new.c -o parallel_scaling -DN=$N
srun -n $P ./parallel_scaling
```

```bash
Processes | Matrix Size (N) | Runtime (s)
-----------------------------------------
        1 |            1000 | 0.002424
        2 |            1414 | 0.005317
        4 |            2000 | 0.007833
        8 |            2832 | 0.019309
       16 |            4000 | 0.042556
```

**Analysis:**
The weak scaling on Dardel shows a consistent and steep increase in runtime from 0.0024s (1 process) to 0.0425s (16 processes). Since we limited the processes to 4 per node (`--ntasks-per-node=4`), the 16-process job was forced to execute across 4 distinct physical nodes. The sharp increase in time—especially when jumping from 2 nodes (8 processes) to 4 nodes (16 processes)—is caused by the heavy inter-node communication penalty over the network. Network communication is significantly slower than intra-node shared memory communication, severely exacerbating the overhead of the MPI collectives.

### Profiling Evaluation

To investigate the poor weak scaling performance on Dardel and find the actual bottleneck, we instrumented the code using Cray Perftools at 16 processes. The following commands were used to compile, instrument, execute, and generate the profiling reports:

```bash
module load perftools-base
module load perftools
cc -O3 hw4-mpi-coll-TBC-new.c -o parallel_scaling -DN=4000
pat_build -g mpi -o parallel_scaling_trace parallel_scaling
srun -n 16 ./parallel_scaling_trace
pat_report -O mpi_callers ./parallel_scaling_trace+*/xf-files > mpi_report.txt
pat_report -O profile ./parallel_scaling_trace+*/xf-files > time_report.txt
```

**MPI Message Stats by Caller (from `mpi_callers` report):**
```text
Table 1:  MPI Message Stats by Caller

    MPI |     MPI Msg |   MPI | MsgSz | 256<= | 1MiB<= | Function
    Msg |       Bytes |   Msg |   <16 | MsgSz |  MsgSz |  Caller
 Bytes% |             | Count | Count | <4KiB | <16MiB |   PE=[mmm]
        |             |       |       | Count |  Count | 
       
 100.0% | 8,002,008.0 |   3.0 |   1.0 |   1.0 |    1.0 | Total
|-------------------------------------------------------------------
| 100.0%| 8,000,000.0 |   1.0 |   0.0 |   0.0 |    1.0 | MPI_Scatter
```

From the MPI callers table, we see that exactly 100% of the MPI Message bytes (8 MB) are attributed to the `MPI_Scatter` operation. Because we are scattering a large $4000 \times 4000$ matrix (at 16 processes) to all nodes, the data distribution creates a massive communication payload over the network. 

**Profile by Function (from `time_report` report):**
```text
Table 1:  Profile by Function Group and Function

  Time% |     Time | Calls | Group / Function
        |          |       |   
 100.0% | 0.062364 |  19.0 | Total
|-----------------------------------------------------------------
|  64.1% | 0.039964 |  11.0 | MPI
||  63.9% | 0.039842 |   1.0 | MPI_Scatter
|=================================================================
|  32.3% | 0.020137 |   7.0 | MPI_SYNC
||  21.6% | 0.013452 |   2.0 | MPI_Barrier(sync)
||   6.5% | 0.004058 |   1.0 | MPI_Finalize(sync)
||   3.2% | 0.001985 |   1.0 | MPI_Gather(sync)
|=================================================================
|   3.6% | 0.002263 |   1.0 | USER
||   3.6% | 0.002263 |   1.0 | main
```

The time profile strongly supports our hypothesis regarding the scaling performance drop. Out of the total execution time recorded by the profiler:
1. **MPI Communication (`MPI_Scatter`):** Takes up roughly **64%** of the entire execution time. Sending the large matrix chunks to different nodes across the network is the primary bottleneck.
2. **MPI Synchronization:** Takes up **32.3%** of the time, primarily waiting at `MPI_Barrier` and `MPI_Gather`.
3. **Actual Computation (`USER/main`):** The mathematical computation (calculating the row sums) takes an astonishingly small **3.6%** of the execution time (0.0022 seconds). 

**Conclusion:** 
The profiling data conclusively shows that this parallelized matrix row-summation program is heavily communication-bound. Nearly 96% of the program's runtime is spent distributing data, waiting for synchronization, and collecting results, while less than 4% is spent doing actual mathematics. This fully explains why our weak scaling efficiency dropped as we added more processes across different nodes.