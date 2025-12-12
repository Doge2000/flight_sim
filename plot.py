import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("sim.csv")

plt.figure()
plt.plot(df["time"], df["alt"], label="Altitude (m)")
plt.xlabel("Time (s)")
plt.ylabel("Altitude (m)")
plt.title("Rocket Altitude Over Time")
plt.show()

plt.figure()
plt.plot(df["time"], df["v"], label="Velocity (m/s)", color='orange')
plt.xlabel("Time (s)")
plt.ylabel("Velocity (m/s)")
plt.title("Rocket Velocity Over Time")
plt.show()
