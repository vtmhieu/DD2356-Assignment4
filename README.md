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

# Exercise 3: