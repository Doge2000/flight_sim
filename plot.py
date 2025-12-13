import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("sim.csv")

# Plot Altitude and Velocity over Time
plt.figure()
plt.plot(df["Time"], df["Altitude"], label="Altitude (m)", color='red')
plt.xlabel("Time (s)")
plt.ylabel("Altitude (m)")
plt.title("Rocket Altitude Over Time")
plt.show(block = False)

# Plot Velocity over Time
plt.figure()
plt.plot(df["Time"], df["Velocity"], label="Velocity (m/s)", color='orange')
plt.xlabel("Time (s)")
plt.ylabel("Velocity (m/s)")
plt.title("Rocket Velocity Over Time")
plt.show()
