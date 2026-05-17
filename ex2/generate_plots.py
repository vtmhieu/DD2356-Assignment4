import numpy as np
import matplotlib.pyplot as plt

def generate_plot(data_file, title, output_file):
    data = np.loadtxt(data_file)
    plt.figure()
    plt.plot(data, marker='o', markersize=2, linestyle='-', label='Row Sums')
    plt.xlabel("Row Index")
    plt.ylabel("Sum Value")
    plt.title(title)
    plt.legend()
    plt.savefig(output_file)
    plt.close()

if __name__ == "__main__":
    generate_plot("row_sums_serial.txt", "Serial Row Sum Computation", "plot_serial.png")
    generate_plot("row_sums_parallel.txt", "Parallel Row Sum Computation with MPI", "plot_parallel.png")
