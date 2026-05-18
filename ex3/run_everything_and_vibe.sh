# compile the versions
gcc hw4-mpi-gameoflife-TBC_serial.c -o serial.out -DWRITE_OUTPUT
mpicc hw4-mpi-gameoflife-TBC_mpi.c -o mpi.out -DWRITE_OUTPUT

# run the serial version of the code
./serial.out
echo
mpirun -np 4 mpi.out

echo
# print that if there is a difference between the two outputs, smth went bad
echo "Checking for differences between serial and MPI outputs"
diff output_text/output_serial/gol_output_99.txt output_text/output_mpi/gol_output_99.txt
if [ $? -eq 0 ]; then
    echo "No diff existent"
else
    echo "Difference found, mand that is bad"
fi