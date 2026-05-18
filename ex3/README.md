# Exercise 3: 2D Game of Life with MPI and Non-Blocking Communication

Conway’s Game of Life is a cellular automaton with simple rules leading to complex behaviors. Your task is to parallelize the Game of Life using MPI with 2D ghost cell exchange and non-blocking communication.

## Running
A little tldr for running
### Serial

```bash
gcc hw4-mpi-gameoflife-TBC_serial.c -o serial.out -DWRITE_OUTPUT
./serial.out
```

```bash
mpicc hw4-mpi-gameoflife-TBC_mpi.c -o mpi.out -DWRITE_OUTPUT
mpirun -np <num_threads> mpi.out
```

Afterwards, for a nice visualization, run 
```bash
python viz.py
```
And modify:
```python
# files = sorted(glob.glob("output_text/output_serial/gol_output_*.txt"))
files = sorted(glob.glob("output_text/output_mpi/gol_output_*.txt"))
```
By commenting and decommenting the place to take files from

- ## Parallelize the serial code using MPI based on the serial version here Download the serial version here:
  the code for this exercise can be seen inside the `hw4-mpi-gameoflife-TBC_mpi.c` file. Over there, paralelization for the opdate function was done
  - ### Decompose the 2D grid into subdomains assigned to MPI processes.
  The decomposition was done so that each process would get a certain number of rows. Initially, there are `N` rows. Each process will receive `N / nr_proc` rows. With these rows, we will attach 2 more rows, which are called ghost_rows. These rows will be holding the information gotten by the processes above and under.
  ```c
  void initialize_subarray(int world_size) {
    // initialize data for every vector
    // Take into consideration the ghost cells
    sub_grid = (int*)malloc((N/world_size + 2) * N * sizeof(int));
    sub_grid_new = (int*)malloc((N/world_size + 2) * N * sizeof(int));
  }
  ```
  - ### Use MPI_Isend and MPI_Irecv for non-blocking ghost cell exchange.
  For this, we are `rascals`, hence we are first sending the data for each process to their neighboring ones
  ```c
  MPI_Isend(sub_grid + N, N, MPI_INT, (rank + world_size - 1) % world_size, 0, MPI_COMM_WORLD, &req[0]);
  // send to the lower guys the lower ghost
  MPI_Isend(sub_grid + N * (N/world_size), N, MPI_INT, (rank + world_size + 1) % world_size, 1, MPI_COMM_WORLD, &req[2]);
  ```
  Afterwards, each process is doing their computation on the sub_grid. (as it doesn't depend on the rows above and beyond):
  ```c
  for (int i = 2; i < N / world_size; i++)
  ```
  And afterwards, each process waits for the messages from their neighbors to arrive:
  ```c
  // receive from the upper guy it's lower ghost
  MPI_Irecv(sub_grid, N, MPI_INT, (rank + world_size - 1) % world_size, 1, MPI_COMM_WORLD, &req[1]);
  // receive from lower guy it's higher ghost
  MPI_Irecv(sub_grid + N * (N/world_size) + N, N, MPI_INT, (rank + world_size + 1) % world_size, 0, MPI_COMM_WORLD, &req[3]);
  ```
  After this data arrives, each process does the grid computation for it's first and last rows:
  ```c
  // now that we have the boundaries, we can compute the upper place
  update_row(1, world_size);
  // now that we have the boundaries, we can compute the and lower place
  update_row(N / world_size, world_size);
  ```
  - ### In the report, briefly describe your parallelization strategy.
  Our paralelization strategy was composed of dividing the matrix into P subsections, assigning each process one. Each process shares the piece of data that is being used by other processes as well at the beginning of an iteration, afterwards starts working on it's parts of data that don't require information owned by other processes. After it finishes, it looks inside it's buffer to see if the data that is required arrived. Afterwards, it uses that data in order to update the first and last rows (as the data there is using data from other processes).
- ## Update cell states based on Conway’s rules:
  - ### A live cell with fewer than two or more than three neighbors dies.
  Already done on the serial version
  - ### A dead cell with exactly three neighbors becomes alive.
  ALready done on the serial version
- ## Visualize the results with Python.
  - ### In the report, present the visualization and validate the correctness of your implementation.
  The correctness (although we are using Wait intrinsic), is done by comparing the output_text/output_serial/gol_output_99.txt with output_text/output_mpi/gol_output_99.txt. If these 2 files are identical (as we are using the same srand seed), that means that the code worked and the paralelization is fine.
  There is even a script for running both of them and checking with diff! Cause we are very nice guys!
  ```bash
  chmod +x run_everything_and_vibe.sh
  ./run_everything_and_vibe.sh
  ```
- ## Analyze the parallel efficiency (on the school cluster) at increased number of processes. Hint: You may want to increase the problem size (N) and the number of steps for better scalability. You may also want to disable the I/O for measuring scalability. 
  The visualization is already deactivated by not inputing the `-DWRITE_OUTPUT` flag. The N is `2000` and the step is `300`. 