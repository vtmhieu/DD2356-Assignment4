#!/bin/bash

# Array of process counts
PROCS=(1 2 4 8 16)
# Base matrix size for 1 process
BASE_N=1000

echo "Running Weak Scaling Experiment..."
echo "Processes | Matrix Size (N) | Runtime (s)"
echo "-----------------------------------------"

for p in "${PROCS[@]}"; do
    # Calculate N for weak scaling: N = BASE_N * sqrt(P)
    # We use awk to compute the square root and round to nearest integer
    N=$(awk -v p=$p -v base=$BASE_N 'BEGIN { printf "%.0f", base * sqrt(p) }')
    
    # Ensure N is divisible by p for MPI_Scatter (otherwise it fails in this simple code)
    # If N is not divisible by p, we adjust it
    REMAINDER=$(( N % p ))
    if [ $REMAINDER -ne 0 ]; then
        N=$(( N - REMAINDER + p ))
    fi

    # Compile with the new N
    mpicc -O3 hw4-mpi-coll-TBC-new.c -o parallel_scaling -DN=$N
    
    # Run and extract time taken
    # Note: capturing output to parse time.
    OUTPUT=$(mpirun -np $p ./parallel_scaling)
    
    # Extract the line with "Time taken"
    TIME=$(echo "$OUTPUT" | grep "Time taken" | awk '{print $3}')
    
    printf "%9d | %15d | %s\n" $p $N "$TIME"
done
