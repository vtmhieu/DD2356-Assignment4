#!/bin/bash
#SBATCH --job-name=mpi_scorep
#SBATCH --time=00:10:00
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=4
#SBATCH --partition=main
#SBATCH --account=edu26.dd2356

# Load necessary modules
module load PDCOLD/23.03
# module load openmpi
module load scorep

# Profiling at the largest scale: 16 processes
P=16
BASE_N=1000

# Calculate N for weak scaling
N=$(awk -v p=$P -v base=$BASE_N 'BEGIN { printf "%.0f", base * sqrt(p) }')
REMAINDER=$(( N % P ))
if [ $REMAINDER -ne 0 ]; then
    N=$(( N - REMAINDER + P ))
fi

# Compile with scorep wrapper
scorep cc -O3 hw4-mpi-coll-TBC-new.c -o parallel_scorep -DN=$N

# Ensure a clean profiling directory
export SCOREP_EXPERIMENT_DIRECTORY=scorep_mpi_run
rm -rf $SCOREP_EXPERIMENT_DIRECTORY

# Run the profiled executable
srun -n $P ./parallel_scorep

# Display the Score-P summary
scorep-score $SCOREP_EXPERIMENT_DIRECTORY/profile.cubex
