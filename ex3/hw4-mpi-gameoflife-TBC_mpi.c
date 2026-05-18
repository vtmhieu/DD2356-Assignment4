#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <string.h>

#define N 2000  // Grid size
#define STEPS 300  // Simulation steps

int grid[N][N], new_grid[N][N];
int* sub_grid_new;
int* sub_grid;

void initialize() {
    unsigned int seed = 42;
    srand(seed);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            
            grid[i][j] = rand() % 2;
        }
    }
}


void free_memory_subarray(int world_size) {
    free(sub_grid);
    free(sub_grid_new);
}

void initialize_subarray(int world_size) {
    // initialize data for every vector
    // Take into consideration the ghost cells
    sub_grid = (int*)malloc((N/world_size + 2) * N * sizeof(int));
    sub_grid_new = (int*)malloc((N/world_size + 2) * N * sizeof(int));
}

// the count_neighbors will be for each subprocess
int count_neighbors(int x, int y, int world_size) {
    int sum = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            int nx = x + i;
            int ny = (y + j + N) % N;
            sum += sub_grid[nx * N + ny];
        }
    }
    return sum;
}

// function for updating a certain row
void update_row(int i, int world_size) {
    for (int j = 0; j < N; j++) {
        int neighbors = count_neighbors(i, j, world_size);
        if (sub_grid[i * N + j] == 1 && (neighbors < 2 || neighbors > 3)) {
            sub_grid_new[i * N + j] = 0;
        } else if (sub_grid[i * N + j] == 0 && neighbors == 3) {
            sub_grid_new[i * N + j] = 1;
        } else {
            sub_grid_new[i * N + j] = sub_grid[i * N + j];
        }
    }
}

void update(int world_size, int rank) {
    MPI_Request req[4];
    // first of all, send the up part to the upper process
    MPI_Isend(sub_grid + N, N, MPI_INT, (rank + world_size - 1) % world_size, 0, MPI_COMM_WORLD, &req[0]);
    // send to the lower guys the lower ghost
    MPI_Isend(sub_grid + N * (N/world_size), N, MPI_INT, (rank + world_size + 1) % world_size, 1, MPI_COMM_WORLD, &req[2]);

    for (int i = 2; i < N / world_size; i++) {
        update_row(i, world_size);
    }

    // receive from the upper guy it's lower ghost
    MPI_Irecv(sub_grid, N, MPI_INT, (rank + world_size - 1) % world_size, 1, MPI_COMM_WORLD, &req[1]);
    // receive from lower guy it's higher ghost
    MPI_Irecv(sub_grid + N * (N/world_size) + N, N, MPI_INT, (rank + world_size + 1) % world_size, 0, MPI_COMM_WORLD, &req[3]);

    // wait for all the sending and receiving operations to finish
    MPI_Waitall(4, req, MPI_STATUSES_IGNORE);

    // now that we have the boundaries, we can compute the upper place
    update_row(1, world_size);
    // now that we have the boundaries, we can compute the and lower place
    update_row(N / world_size, world_size);
    
    
    for (int i = 1; i < N / world_size + 1; i++) {
        for (int j = 0; j < N; j++) {
            sub_grid[i * N + j] = sub_grid_new[i * N + j];
        }
    }
}

#ifdef WRITE_OUTPUT
void write_output(int step) {
    char filename[100];
    char foldername[100] = "output_text/output_mpi/";
    sprintf(filename, strcat(foldername, "gol_output_%d.txt"), step);
    FILE *f = fopen(filename, "w");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(f, "%d ", grid[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}
#endif

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // if(world_size < 2 && rank == 0) {
    //     printf("MPI Program didn't work as there are too few processes\n");
    //     MPI_Abort(MPI_COMM_WORLD, 1);
    // }
    if (N % world_size != 0 && rank == 0) {
        printf("Give a number of processes divisible with %d, in order to have a nice slit of the matrix\n", N);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // if we are rank 0 let's directly split up the work to everybody
    if(rank == 0) {
        initialize();
    }

    initialize_subarray(world_size);


    // split it to everybody else
    // also matrices in C are contiguous which is AWESOME
    int SUBARRAY_SIZE = N * N / world_size;
    MPI_Scatter(grid, SUBARRAY_SIZE, MPI_INT, sub_grid + N, SUBARRAY_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
    
    /*TODO: Timer begins */

    double start = MPI_Wtime();
    for (int step = 0; step < STEPS; step++) {
        update(world_size, rank);
        #ifdef WRITE_OUTPUT
        if (step % 1 == 0) {
            MPI_Gather(sub_grid + N, SUBARRAY_SIZE, MPI_INT, grid, SUBARRAY_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
            if (rank == 0)
                write_output(step);
        }
        #endif
    }
    double end = MPI_Wtime();
    double local_seconds = (end - start);
    double max_time = 0;
    /*TODO: Timer ends */
    MPI_Reduce(&local_seconds, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    free_memory_subarray(world_size);
    
    MPI_Finalize();
    if (rank == 0) {
        printf("The game is over.\n");
        printf("The game lasted: %f seconds", max_time);
    }
    return 0;
}
