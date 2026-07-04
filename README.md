# Rocket Flight Simulator

A physics-based multi-stage rocket simulator modeling thrust curves, drag, and parachute deployment.
Comes with two interfaces: a legacy matplotlib GUI and a live web-based telemetry dashboard.

# Demos 
- Legacy
(0624.gif)
- Web-based
(webbased.gif)

## Features
- Multi-stage separation with configurable thrust, fuel mass, dry mass, and burn time per stage
- Drag modeling with separate coefficients for freefall and parachute descent
- Parachute deployment triggered below 200m on descent
- Automatic pitch program after liftoff
- Real-time X/Y trajectory and altitude visualization

## Web Dashboard (recommended)
The web version streams live telemetry from the C++ sim to a browser dashboard via WebSockets.

### How to Run
1. Compile: `g++ main.cpp -o sim.exe`
2. Start server: `python -m uvicorn server:app --reload`
3. Open `localhost:8000` in your browser
4. Configure stage parameters and click **Start**

### Stack
- **Backend:** Python, FastAPI, WebSockets
- **Frontend:** HTML/JS, Plotly.js
- **Simulation:** C++

## Legacy Matplotlib Version
The original batch-mode version runs the full simulation first, then animates the result.

### How to Run
1. Compile: `g++ main.cpp -o sim.exe`
2. Run: `python plot.py`

## Architecture
The C++ simulation engine writes telemetry to stdout line by line. The Python FastAPI server
reads this stream via subprocess and forwards each row to the browser over a WebSocket connection.
The browser plots incoming data points in real time using Plotly.js.

```
C++ sim → stdout → FastAPI server → WebSocket → Browser (Plotly.js)
```

## Tech Stack
C++, Python, FastAPI, Plotly.js, HTML/JS