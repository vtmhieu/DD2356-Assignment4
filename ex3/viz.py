import numpy as np
import matplotlib.pyplot as plt
import glob

# files = sorted(glob.glob("output_text/output_serial/gol_output_*.txt"))
files = sorted(glob.glob("output_text/output_mpi/gol_output_*.txt"))

for file in files:
    data = np.loadtxt(file)
    plt.imshow(data, cmap='binary', origin='lower')
    plt.title(f"Game of Life - {file}")
    plt.pause(0.5)

plt.show()