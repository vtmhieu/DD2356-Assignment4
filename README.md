# Exercise 1:

To compile the code locally/school cluster: 

```
mpicc -o parallel_halo parallel_halo.c -lm
```

To compile the code on dardel: 
```
cc -o parallel_halo parallel_halo.c -lm
```

To run the code locally/on school cluster with a certain number of processes: 

```
mpirun -np {N} ./parallel_halo
```

To run the code on dardel use a sample job file: 

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

and run: 

```
sbatch myjob.sh
```

# Exercise 2:

Compiling and executing with P processes Locally/School Cluster: 

```bash
mpicc -O3 hw4-mpi-coll-TBC-new.c -o parallel_scaling -DN=$N
mpirun -np $P ./parallel_scaling
```

For Dardel: 
The job was submitted using a Slurm batch script (`sbatch dardel_weak_scaling.sh`) configured with `--ntasks-per-node=4`. Inside the script, the code was compiled with the standard Cray compiler wrapper and executed using `srun`:
```bash
cc -O3 hw4-mpi-coll-TBC-new.c -o parallel_scaling -DN=$N
srun -n $P ./parallel_scaling
```

# Exercise 3:

A little tldr for compiling/running 
### Serial

```bash
gcc hw4-mpi-gameoflife-TBC_serial.c -o serial.out -DWRITE_OUTPUT
./serial.out
```

### MPI

```bash
mpicc hw4-mpi-gameoflife-TBC_mpi.c -o mpi.out -DWRITE_OUTPUT
mpirun -np <num_threads> mpi.out
```

Afterwards, for a nice visualization, run 
```bash
python viz.py
```