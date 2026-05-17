import matplotlib.pyplot as plt

# Replace these lists with the actual runtime data obtained from the scripts
procs = [1, 2, 4, 8, 16]
# Example runtimes (replace with real output)
times_school = [0.001574, 0.003270, 0.008479, 0.016070, 0.006481] 
times_dardel = [0.002424, 0.005317, 0.007833, 0.019309, 0.042556]

# Plot for School Cluster
plt.figure(figsize=(8, 6))
plt.plot(procs, times_school, marker='o', color='blue', linestyle='-')
plt.xlabel("Number of Processes")
plt.ylabel("Execution Time (s)")
plt.title("Weak Scaling Performance - School Cluster")
plt.xticks(procs)
plt.grid(True, which="both", ls="--")
plt.savefig("weak_scaling_school.png")
print("Saved plot to weak_scaling_school.png")
plt.close()

# Plot for Dardel
plt.figure(figsize=(8, 6))
plt.plot(procs, times_dardel, marker='s', color='orange', linestyle='-')
plt.xlabel("Number of Processes")
plt.ylabel("Execution Time (s)")
plt.title("Weak Scaling Performance - Dardel")
plt.xticks(procs)
plt.grid(True, which="both", ls="--")
plt.savefig("weak_scaling_dardel.png")
print("Saved plot to weak_scaling_dardel.png")
plt.close()
