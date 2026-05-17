#include <stdio.h>
#include <stdlib.h>

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

int main() {
  double matrix[N][N], row_sums[N];
  initialize_matrix(matrix);
  /*TODO: Timer begins */
  compute_row_sums(matrix, row_sums);
  /*TODO: Timer ends */
  printf("Row sum computation complete.\n");
  return 0;
}
