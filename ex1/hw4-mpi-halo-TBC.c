#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define N 1000 // Grid size
#define STEPS 100 // Time steps
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

void compute_step() {
    double u_next[N];
    for (int i = 1; i < N - 1; i++) {
        u_next[i] = 2.0 * u[i] - u_prev[i] + C * C * DT * DT / (DX * DX) * (u[i+1] - 2.0 * u[i] + u[i-1]);
    }
    for (int i = 0; i < N; i++) {
        u_prev[i] = u[i];
        u[i] = u_next[i];
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
    printf("MPI rank %d out of %d processes.\n", rank, nprocs);

    initialize();
    /*TODO: Timer begins */
    for (int step = 0; step < STEPS; step++) {
        compute_step();
        if (step % 10 == 0) write_output(step);
    }
    /*TODO: Timer ends  */
    MPI_Finalize();

    if(rank==0)
      printf("Simulation complete.\n");
    return 0;
}