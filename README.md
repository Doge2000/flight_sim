# Rocket Flight Simulator

A physics-based two-stage rocket simulator modeling real thrust curves, drag coefficients, and parachute deployment, with a live matplotlib GUI for interactive flight configuration.

## Physics Model
- Thrust curves and mass flow rate
- Drag force with configurable Cd
- X/Y velocity and acceleration components
- Parachute deployment at 200 meters

## How to Run
1. Compile: `g++ main.cpp -o sim.exe`
2. Run: `./sim`
3. Visualize: `python plot.py`

## Output
![alt text](0624.gif)