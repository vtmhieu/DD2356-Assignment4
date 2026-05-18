#include <stdio.h>
#include <stdlib.h>
// #include <mpi.h>
#include <time.h>
#include <string.h>

#define N 2000  // Grid size
#define STEPS 300  // Simulation steps

int grid[N][N], new_grid[N][N];

void initialize() {
    unsigned int seed = 42;
    srand(seed);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            grid[i][j] = rand() % 2;  // Random initial state
        }
    }
}

int count_neighbors(int x, int y) {
    int sum = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            int nx = (x + i + N) % N;
            int ny = (y + j + N) % N;
            sum += grid[nx][ny];
        }
    }
    return sum;
}

void update() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int neighbors = count_neighbors(i, j);
            if (grid[i][j] == 1 && (neighbors < 2 || neighbors > 3)) {
                new_grid[i][j] = 0;
            } else if (grid[i][j] == 0 && neighbors == 3) {
                new_grid[i][j] = 1;
            } else {
                new_grid[i][j] = grid[i][j];
            }
        }
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            grid[i][j] = new_grid[i][j];
        }
    }
}

#ifdef WRITE_OUTPUT
void write_output(int step) {
    char filename[100];
    char foldername[100] = "output_text/output_serial/";
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
    // MPI_Init(&argc, &argv);
    
    initialize();
    
    /*TODO: Timer begins */

    clock_t start = clock();
    for (int step = 0; step < STEPS; step++) {
        update();
        #ifdef WRITE_OUTPUT
        if (step % 1 == 0) write_output(step);
        #endif
    }
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    /*TODO: Timer ends */
    
    // MPI_Finalize();
    printf("The game is over.\n");
    printf("The game laster: %f seconds", seconds);
    return 0;
}
