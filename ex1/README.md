# Exercise 1: 1D Halo Exchange in a Wave Equation Simulation

## Parallelization Strategy:

To parallelize the code, at each step, each process will take one part of the 1D array to compute the next values for, and will also update the previous and current arrays in its assigned section. This is done by using MPI_Scatter to have the root process (rank 0) send distinct portions of the original u and u_prev matrices to all processes to store in local arrays that they will perform their updates on. 

However, the computations for the next step rely on the value both preceding and suceeding it in the array, which means if we just send each section in isolation to processes, then for values on the left most boundary - they will not have the neccessary preceding value to perform the next step computation (and same for the right most boundary value). To combat this, we use a halo exchange, where we have processes communicate with each other to retrieve thses neccessary values before going forwar with computation on their section. 

We do this by having the local arrays have a buffer of 1 index on both sides, and using MPI_Sendrecv to have processes send their boundary values to their neighbor processes (the process with rank one lower and one higher than the sending process), and in this way we populate each process with the neccessary boundary values to perform their computation. We only need to check for the process with rank 0 and rank (number of processes - 1) as they cannot send their left and right most boundary values to another process. 

For verification purposes, we then have a gather to update the global arrays at every 10th step by then reducing all of the local sections (ignoring each local halo values at the boundaries of the local arrays) back into the current main array. From the output files written at every 10th step, we are able to visualize the wave simulation and compare the sequential and parallel versions to verify that the program execution is not altered.

## Verification

Sequential: ![Sequential Wave Simulation Visualization](SequentialOutput.png)

Parallel: ![MPI Parallel Wave Simulation Visualization](ParallelOutput.png)

We can see that the visualization for running the two different versions of this program with the same parameters gives the same visualization, showing that the computation is not affected by our parallelization, and the parallel implementation is correct. With the verification of the correctness of the code, when evaluating, we remove the I/O at every 10th step to properly measure scalability, as well as increase our grid size dramatically (from 1000 to 1000000) and the number of steps (from 100 to 10000).

## School Cluster Evaluation

To compile the code on the school cluster: mpicc -o parallel_halo parallel_halo.c -lm

To run the code on the school cluster with a certain number of processes: mpirun -np {N} ./parallel_halo

Runtime:
| Processes | Time (s) |
|---------|----------|
| 1       | 39.146   |
| 2       | 20.05    |
| 4       | 9.96     |
| 8       | 5.47     |
| 16      | 7.86     |


Strong Scaling: ![Strong Scaling School Cluster](SchoolCluster-StrongScaling.png)

We can see that the strong scaling performs very well on the school cluster, even when compared to the ideal. From processes 2-8, the strong scaling nears the linear improvement of the ideal, which shows that the parallel version of the code reaches near full capability. However, we notice a signficant performance drop at 16 processes, both in the strong scaling, and in the actual runtime taking longer than at 8 processes. This may be due to the halo exchange now requiring much more communication at 16 processes to exchange boundary values, which may now overwhelm the added parallelization benefits of segmenting the array into smaller sections for each process. On a larger array, 16 processes may still provide added performance benefit as the computation of the process may not be under saturated with too small a section for the communication costs.

## Dardel Evaluation

To compile the code on dardel: cc -o parallel_halo parallel_halo.c -lm

To run the code on dardel we used a batch job shell script, to run with a certain number of processes P, adjust the --ntasks-per-node to P and the srun command to have -n {P}. To schedule our job on dardel we simply do sbatch {job shell file}

Our job file:

```
#!/bin/bash
#SBATCH --job-name=mpi_parallel_halo
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=16
#SBATCH --partition=main
#SBATCH --account=edu26.dd2356
#SBATCH -e error_file.e


# Load necessary modules (adjust based on Dardel's actual modules)
# module load PDC/23.03
# module load openmpi

srun -n 16 ./parallel_halo
```


Runtime:
| Processes | Time (s) |
|---------|----------|
| 1       | 63.13    |
| 2       | 31.70    |
| 4       | 16.17    |
| 8       | 10.02    |
| 16      | 5.23     |

Strong Scaling: ![Strong Scaling Dardel](Dardel-StrongScaling.png)

We can see that the strong scaling also performs very well on Dardel, acheiving similar strong performance scaling nearing the ideal linear strong scaling as we increase the process count. We notice a slighly more measurable dip in the strong scaling for 8 processes, as on the school cluster we had a speedup of 7.16 while on Dardel it is just 6.03. However we notice that unlike the school cluster the speedup increases continue on Dardel for 16 processes. On dardel, the speedup from 8 to 16 nearly doubles - which is the ideal possible improvement, while the school cluster had the speedup take a signifcant drop. Despite the raw execution time taking longer on Dardel for process counts of 1-8, we can achieve our minimum execution time across all runs on both systems with a process count of 16 on Dardel, with the strong scaling still imrpvoing at a close to ideal rate. Further performance improvements are possible on Dardel with higher process counts or on larger scale problem sizes. Overall, the scalability of the parallel code extends further on Dardel than the school cluster.

ScoreP Evaluation: 

To be able to run score-P tracing on our parallel_halo code, we had to do some additional setup, namely after compiling the original .c code, running:

```
pat_build -g mpi -o parallel_halo_trace ./parallel_halo 
```

This is to be able to run the executable while collecting the Score-P performance data of the execution. Afterwards, a directory is created to hold the performance reports, where to observe the relevant results we run: 

```
pat_report -O mpi_callers ./parallel_halo_trace+*/xf-files

pat_report -O profile ./parallel_halo_trace+*/xf-files
```

When checking the normal output job file, we noticed a similar ~5 seconds runtime when running with performance tracing, showing that the execution itself was not impacted in any way by the performance tracking and that the results are accurate for a normal execution of the parallel code.

For the mpi_callers report, the relevant section is:

Table 1: MPI Message Stats by Caller

MPI | MPI Msg | MPI Msg | MsgSz | 64KiB<= | Function
Msg | Bytes | Count | <16 | MsgSz | Caller
Bytes% | | | Count | <1MiB | PE=[mmm]
| | | | Count |

100.0% | 1,150,000.0 | 18,752.0 | 18,750.0 | 2.0 | Total
|--------------------------------------------------------------------
| 87.0% | 1,000,000.0 | 2.0 | 0.0 | 2.0 | MPI_Scatter
| | | | | | main
|||------------------------------------------------------------------
3|| 87.0% | 1,000,000.0 | 2.0 | 0.0 | 2.0 | pe.0
3|| 87.0% | 1,000,000.0 | 2.0 | 0.0 | 2.0 | pe.8
3|| 87.0% | 1,000,000.0 | 2.0 | 0.0 | 2.0 | pe.15
|||==================================================================
| 13.0% | 150,000.0 | 18,750.0 | 18,750.0 | 0.0 | MPI_Sendrecv
| | | | | | halo_exchange
3 | | | | | main
||||-----------------------------------------------------------------
4||| 13.9% | 160,000.0 | 20,000.0 | 20,000.0 | 0.0 | pe.1
4||| 13.9% | 160,000.0 | 20,000.0 | 20,000.0 | 0.0 | pe.9
4||| 7.0% | 80,000.0 | 10,000.0 | 10,000.0 | 0.0 | pe.15
|====================================================================

In total, on an execution with 16 processes, we make 18,752 MPI calls throughout. We can see from the table, that we have only 2 MPI_Scatter calls, which aligns with our program as MPI_Scatter is only done by the root process with rank 0, and is done once for the two global arrays (u and u_prev). The rest of the 18,750 calls done are MPI_Sendrecv calls that are made during the halo exchanges between processes (we have no MPI_Gather calls since we removed the I/O during performance evaluation).

For the profile report, the relevant section is: 

Table 1: Profile by Function Group and Function

Time% | Time | Imb. | Imb. | Calls | Group
| | Time | Time% | | Function
| | | | | PE=HIDE

100.0% | 4.997868 | -- | -- | 18,767.0 | Total
|--------------------------------------------------------------
| 79.2% | 3.958183 | 0.883247 | 19.5% | 1.0 | USER
||-------------------------------------------------------------
|| 79.2% | 3.958183 | 0.883247 | 19.5% | 1.0 | main
||=============================================================
| 20.4% | 1.021526 | -- | -- | 18,760.0 | MPI
||-------------------------------------------------------------
|| 20.1% | 1.006466 | 0.094513 | 9.2% | 18,750.0 | MPI_Sendrecv
|==============================================================

From the table, we can see that from the ~5 seconds (4.997) total execution time, around 1.02 seconds is spent on MPI calls, with the rest of the time spent elsewhere. With the vast majority of the MPI calls being MPI_Sendrecv, it makes sense that an overwhelming portion of the time spent during MPI calls is on MPI_Sendrecv (~1 of the 1.02 seconds). The two MPI_Scatter calls do make up a larger proportion of the total MPI call time than the total MPI calls made, and this may be due to MPI_Scatter being a much more complicated communication operation than a simpler MPI_Sendrecv.



