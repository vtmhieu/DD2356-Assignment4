#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define N 1000000 // Grid size
#define STEPS 10000 // Time steps
#define C 1.0   // Wave speed
#define DT 0.01 // Time step
#define DX 1.0  // Grid spacing

double u[N], u_prev[N];

void initialize() {
    for (int i = 0; i < N; i++) {
        u[i] = sin(2.0 * M_PI * i / N);
        u_prev[i] = u[i];
    }
}

void compute_step(double *local_u, double *local_u_prev, int indexesPerProc) {
    
    double u_next[indexesPerProc];
    for (int i = 1; i < indexesPerProc; i++) {
        u_next[i] = 2.0 * local_u[i] - local_u_prev[i] + C * C * DT * DT / (DX * DX) * (local_u[i+1] - 2.0 * local_u[i] + local_u[i-1]);
    }
    for (int i = 1; i < indexesPerProc; i++) {
        local_u_prev[i] = local_u[i];
        local_u[i] = u_next[i];
    }
}

void halo_exchange(double *local_u, int local_n, int rank, int nprocs) {
    MPI_Status status;

    if (rank > 0) {
        MPI_Sendrecv(&local_u[1], 1, MPI_DOUBLE, rank-1, 0,
                     &local_u[0], 1, MPI_DOUBLE, rank-1, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);
    }

    if (rank < nprocs-1) {
        MPI_Sendrecv(&local_u[local_n], 1, MPI_DOUBLE, rank+1, 1,
                     &local_u[local_n+1], 1, MPI_DOUBLE, rank+1, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);
    }
}

void write_output(int step) {
    char filename[50];
    sprintf(filename, "wave_output_%d.txt", step);
    FILE *f = fopen(filename, "w");
    for (int i = 0; i < N; i++) {
        fprintf(f, "%f\n", u[i]);
    }
    fclose(f);
}

int main(int argc, char** argv) {
    int rank, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (rank == 0) {
        initialize();
    }

    int indexesPerProc = N / nprocs;
    double *local_u      = malloc((indexesPerProc + 2) * sizeof(double));
    double *local_u_prev = malloc((indexesPerProc + 2) * sizeof(double));

    MPI_Scatter(u, indexesPerProc, MPI_DOUBLE, &local_u[1], indexesPerProc, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(u_prev, indexesPerProc, MPI_DOUBLE, &local_u_prev[1], indexesPerProc, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    for (int step = 0; step < STEPS; step++) {
        halo_exchange(local_u, indexesPerProc, rank, nprocs);
        compute_step(local_u, local_u_prev, indexesPerProc);
        // if (step % 10 == 0) {
        //     MPI_Gather(&local_u[1], indexesPerProc, MPI_DOUBLE, u, indexesPerProc, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        //     if (rank == 0){
        //         write_output(step);
        //     }
        // }
    }
    /*TODO: Timer ends  */
    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    MPI_Finalize();

    if(rank==0){
      printf("Simulation complete.\n");
      printf("Total execution time: %f seconds.\n", end_time - start_time);
    }
    return 0;
}