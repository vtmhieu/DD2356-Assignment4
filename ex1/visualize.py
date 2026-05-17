import numpy as np
import matplotlib.pyplot as plt
import glob

files = sorted(glob.glob("wave_output_*.txt"))

for file in files:
    data = np.loadtxt(file)
    plt.plot(data, label=f"Time step {file.split('_')[-1].split('.')[0]}")

plt.xlabel("Grid Point")
plt.ylabel("Wave Amplitude")
plt.title("Wave Equation Evolution")
plt.legend()
plt.savefig("output.png")