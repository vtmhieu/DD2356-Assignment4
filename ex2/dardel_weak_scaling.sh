#!/bin/bash
#SBATCH --job-name=mpi_weak_scaling
#SBATCH --time=00:10:00
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=4
#SBATCH --partition=main
#SBATCH --account=edu24.dd2356

# Load necessary modules (adjust based on Dardel's actual modules)
# module load PDC/23.03
# module load openmpi

PROCS=(1 2 4 8 16)
BASE_N=1000

echo "Running Weak Scaling Experiment on Dardel..."
echo "Processes | Matrix Size (N) | Runtime (s)"
echo "-----------------------------------------"

for p in "${PROCS[@]}"; do
    N=$(awk -v p=$p -v base=$BASE_N 'BEGIN { printf "%.0f", base * sqrt(p) }')
    
    # Ensure N is divisible by p
    REMAINDER=$(( N % p ))
    if [ $REMAINDER -ne 0 ]; then
        N=$(( N - REMAINDER + p ))
    fi

    # Compile with the new N
    cc -O3 hw4-mpi-coll-TBC-new.c -o parallel_scaling -DN=$N
    
    # Run using srun
    OUTPUT=$(srun -n $p ./parallel_scaling)
    
    # Extract time
    TIME=$(echo "$OUTPUT" | grep "Time taken" | awk '{print $3}')
    
    printf "%9d | %15d | %s\n" $p $N "$TIME"
done
