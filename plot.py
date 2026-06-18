import pandas as pd
import subprocess
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Slider, Button, TextBox


def runsim(stage1_thrust, stage2_thrust, stage1_fuel, stage2_fuel, stage1_mass, stage2_mass):
    subprocess.run(["sim.exe", str(stage1_thrust), str(stage2_thrust), str(stage1_fuel), str(stage2_fuel), str(stage1_mass), str(stage2_mass)])

def load_data():
    df = pd.read_csv("sim.csv")
    t   = df["Time"].values
    x   = df["X"].values
    y   = df["Y"].values
    vx  = df["Vx"].values
    vy  = df["Vy"].values
    v   = np.sqrt(vx**2 + vy**2)
    return t, x, y, vx, vy, v

t = np.array([]); x = np.array([]); y = np.array([])
vx = vy = v = np.array([])

fig, (ax_sim, ax_plot) = plt.subplots(1, 2, figsize=(14, 7))
plt.subplots_adjust(left=0.05, right=0.55, bottom=0.3, top=0.75, wspace=0.3)


thrust1inputbox = plt.axes([0.75, 0.25, 0.2, 0.03])
thrust2inputbox = plt.axes([0.75, 0.2, 0.2, 0.03])
fuel1inputbox = plt.axes([0.75, 0.45, 0.2, 0.03])
fuel2inputbox = plt.axes([0.75, 0.4, 0.2, 0.03])
mass1inputbox = plt.axes([0.75, 0.65, 0.2, 0.03])
mass2inputbox = plt.axes([0.75, 0.6, 0.2, 0.03])

thrust1 = TextBox(thrust1inputbox, "Stage 1 Thrust (N)", initial="20.0")
thrust2 = TextBox(thrust2inputbox, "Stage 2 Thrust (N)", initial="20.0")
fuel1 = TextBox(fuel1inputbox, "Stage 1 Fuel (kg)", initial="0.150")
fuel2 = TextBox(fuel2inputbox, "Stage 2 Fuel (kg)", initial="0.150")
mass1 = TextBox(mass1inputbox, "Stage 1 Dry Mass (kg)", initial="0.150")
mass2 = TextBox(mass2inputbox, "Stage 2 Dry Mass (kg)", initial="0.150")

ax_sim.set_title("Rocket Trajectory (X vs Y)")
ax_plot.set_title("Altitude vs Time")
ax_sim.grid(True, linestyle="--", alpha=0.4)
ax_plot.grid(True, linestyle="--", alpha=0.4)

ax_sim.set_xlim(-100, 100); ax_sim.set_ylim(0, 150)
ax_plot.set_xlim(0, 20); ax_plot.set_ylim(-30, 150)

rocket, = ax_sim.plot([], [], 'ro', markersize=8)
trajectory, = ax_sim.plot([], [], 'r--', lw=1)
line, = ax_plot.plot([], [], 'b-')

ax_start = plt.axes([0.65, 0.12, 0.15, 0.05])
start_button = Button(ax_start, 'Start')

ax_pause = plt.axes([0.82, 0.12, 0.15, 0.05])
pause_button = Button(ax_pause, 'Pause/Resume')

ax_reset = plt.axes([0.73, 0.05, 0.15, 0.05])
reset_button = Button(ax_reset, 'Reset')

# Animation state
anim = None
frames_count = 0
started = False
paused = False

def init():
    rocket.set_data([], [])
    trajectory.set_data([], [])
    line.set_data([], [])
    return rocket, trajectory, line

def update(frame):
    i = min(frame, frames_count - 1)
    rocket.set_data([x[i]], [y[i]])
    trajectory.set_data(x[:i+1], y[:i+1])
    line.set_data(t[:i+1], y[:i+1])
    return rocket, trajectory, line

def start_animation(event):
    global t, x, y, vx, vy, v, frames_count, anim, started
    if started: return
    started = True

    runsim(float(thrust1.text), float(thrust2.text), float(fuel1.text), float(fuel2.text), float(mass1.text), float(mass2.text))
    t, x, y, vx, vy, v = load_data()
    frames_count = len(t)

    ax_sim.set_xlim(min(-50, float(np.nanmin(x)) * 1.1),
                    max(50, float(np.nanmax(x)) * 1.1))
    ax_sim.set_ylim(0, max(50, float(np.nanmax(y)) * 1.1))
    ax_plot.set_xlim(0, max(20, float(np.nanmax(t))))
    ax_plot.set_ylim(min(0, float(np.nanmin(y)) * 1.1),
                     max(50, float(np.nanmax(y)) * 1.1))

    init()
    anim = FuncAnimation(fig, update, init_func=init,
                         frames=range(0, frames_count, 50), interval=0.5, blit=True,
                         repeat=False) 
    fig.canvas.draw_idle()

def pause_resume(event):
    global paused
    if anim is None: return
    if paused:
        anim.event_source.start()
        paused = False
    else:
        anim.event_source.stop()
        paused = True

def reset_animation(event):
    global t, x, y, vx, vy, v, frames_count, anim, started, paused
    started = False; paused = False
    t = x = y = vx = vy = v = np.array([])
    frames_count = 0
    init()
    ax_sim.set_xlim(-100, 100); ax_sim.set_ylim(0, 150)
    ax_plot.set_xlim(0, 20); ax_plot.set_ylim(-30, 150)
    fig.canvas.draw_idle()

start_button.on_clicked(start_animation)
pause_button.on_clicked(pause_resume)
reset_button.on_clicked(reset_animation)

plt.show()