import matplotlib.pyplot as plt
import pandas as pd

# Load data
data = pd.read_csv("pid_data.csv")

# Plot
plt.figure(figsize=(8,5))
plt.plot(data["Time"], data["PitchOutput"], label="Pitch Output", marker="o")
plt.plot(data["Time"], data["CurrentPitch"], label="Current Pitch", marker="s")
plt.xlabel("Time (s)")
plt.ylabel("Values")
plt.title("PID Controller Response")
plt.legend()
plt.grid(True)
plt.show()
