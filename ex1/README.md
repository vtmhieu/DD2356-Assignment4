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

