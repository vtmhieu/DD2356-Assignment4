import matplotlib.pyplot as plt

# Replace these lists with the actual runtime data obtained from the scripts
procs = [1, 2, 4, 8, 16]
# Example runtimes (replace with real output)
times_school = [0.01, 0.011, 0.012, 0.014, 0.015] 
times_dardel = [0.008, 0.009, 0.009, 0.011, 0.012]

plt.figure(figsize=(8, 6))

plt.plot(procs, times_school, marker='o', linestyle='-', label='School Cluster')
plt.plot(procs, times_dardel, marker='s', linestyle='--', label='Dardel')

plt.xlabel("Number of Processes")
plt.ylabel("Execution Time (s)")
plt.title("Weak Scaling Performance")
plt.xticks(procs)
plt.legend()
plt.grid(True, which="both", ls="--")

plt.savefig("weak_scaling_plot.png")
print("Saved plot to weak_scaling_plot.png")
