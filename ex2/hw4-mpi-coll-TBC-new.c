#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000 // Matrix size

/*
Modify the serial code to use MPI_Scatter to distribute matrix rows among
processes. Each process computes the row sums of its assigned portion of the
matrix. Use MPI_Gather to collect results back at the root process. Use
MPI_Reduce to compute the total sum of all matrix elements. Add timers in TODOs
using MPI_Wtime() and MPI_Barrier(). In the report, briefly describe your
parallelization strategy.
*/

void initialize_matrix(double matrix[N][N]) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      matrix[i][j] = i + j * 0.01;
    }
  }
}

void compute_row_sums(double matrix[N][N], double row_sums[N]) {
  for (int i = 0; i < N; i++) {
    row_sums[i] = 0.0;
    for (int j = 0; j < N; j++) {
      row_sums[i] += matrix[i][j];
    }
  }
}

void write_output(double row_sums[N]) {
  FILE *f = fopen("row_sums_output.txt", "w");
  for (int i = 0; i < N; i++) {
    fprintf(f, "%f\n", row_sums[i]);
  }
  fclose(f);
}

int main(int argc, char **argv) {
  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  double matrix[N][N], row_sums[N];

  if (rank == 0) {
    initialize_matrix(matrix);
  }

  int rows_per_proc = N / size;
  double local_matrix[rows_per_proc][N];
  double local_row_sums[rows_per_proc];

  /*TODO: Timer begins */
  MPI_Barrier(MPI_COMM_WORLD);
  double start_time = MPI_Wtime();

  // Distribute matrix rows among processes
  MPI_Scatter(matrix, rows_per_proc * N, MPI_DOUBLE, local_matrix,
              rows_per_proc * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Each process computes the row sums of its assigned portion
  for (int i = 0; i < rows_per_proc; i++) {
    local_row_sums[i] = 0.0;
    for (int j = 0; j < N; j++) {
      local_row_sums[i] += local_matrix[i][j];
    }
  }

  // Collect results back at the root process
  MPI_Gather(local_row_sums, rows_per_proc, MPI_DOUBLE, row_sums, rows_per_proc,
             MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Compute the total sum of all matrix elements locally
  double local_total_sum = 0.0;
  for (int i = 0; i < rows_per_proc; i++) {
    local_total_sum += local_row_sums[i];
  }

  // Use MPI_Reduce to compute the total sum across all processes
  double global_total_sum = 0.0;
  MPI_Reduce(&local_total_sum, &global_total_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
             MPI_COMM_WORLD);

  /*TODO: Timer ends */
  MPI_Barrier(MPI_COMM_WORLD);
  double end_time = MPI_Wtime();

  if (rank == 0) {
    write_output(row_sums);
    printf("Row sum computation complete.\n");
    printf("Total sum: %f\n", global_total_sum);
    printf("Time taken: %f seconds\n", end_time - start_time);
  }

  MPI_Finalize();
  return 0;
}
