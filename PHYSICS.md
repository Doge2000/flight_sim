# Physics Model

This document describes the physics implemented in `main.cpp`: the equations of motion, atmospheric and wind models, staging logic, and the flight-phase detection used to drive telemetry status messages. It reflects what the code actually does, including simplifications and a couple of known quirks worth being aware of.

## Integration Scheme

The simulator uses **semi-implicit (symplectic) Euler integration** at a fixed timestep of `dt = 0.0001 s` (10000 Hz), matching the README's stated update rate.

Each step:
1. Compute net force `F` from thrust, drag, and gravity using the *current* state.
2. Update velocity: `v += a * dt`, where `a = F / m`.
3. Update position using the **new** velocity: `x += v * dt`.

This is a step up from plain (explicit) Euler — using the updated velocity for the position update makes the integrator symplectic, which is more energy-stable over long runs than naive Euler, at the cost of being less accurate than something like RK4. For a fixed 0.01 s step and a rocket flight lasting tens of seconds, this is a reasonable trade of simplicity for stability.

State is initialized to `1e-6` (not exactly `0`) for position and velocity components, avoiding degenerate `0/0` cases in downstream calculations.

## Coordinate System

- `y` is altitude (vertical).
- `x`, `z` are horizontal (`x` is the primary direction of travel; `z` is cross-range).
- **Pitch** is the angle from the horizontal plane: `pitch = π/2` (90°) means pointing straight up.
- **Yaw** rotates the horizontal thrust component between `x` and `z`.

Direction of thrust is computed directly from spherical-style trig rather than a normalized vector:
```
thrust_x = T * cos(pitch) * cos(yaw)
thrust_y = T * sin(pitch)
thrust_z = T * cos(pitch) * sin(yaw)
```

## Forces

Three forces act on the rocket at every step:

**Gravity** — constant `g = 9.81 m/s²`, applied only to the vertical axis as `-m * g`. No altitude-dependent gravity falloff (not needed at these altitudes/velocities).

**Thrust** — applied only while the *current* stage has fuel remaining (`stage.fuel > 0`), directed along the current pitch/yaw.

**Drag** — standard drag equation, `F_drag = 0.5 * ρ * v_rel² * A * Cd`, applied opposite the relative velocity vector (velocity relative to wind, not ground velocity). Two different effective `A * Cd` values are used:
- Ballistic (no chute): `area = 0.00113 m²`, `Cd = 0.5`
- Parachute deployed: a fixed effective `A*Cd = 0.46 * 1.5`, hardcoded rather than derived from a separate parachute area/Cd

Net acceleration is `a = F_total / m`, where `m` is the **current total mass** — sum of dry mass + remaining fuel across *all* stages (spent stages contribute zero once jettisoned; unignited upper stages still count, since their mass is physically still attached).

## Atmosphere Model

Air density uses a simple **exponential atmosphere**:
```
ρ(y) = 1.225 * exp(-y / 8500)
```
`1.225 kg/m³` is sea-level density; `8500 m` is the scale height — a standard first-order approximation for Earth's lower atmosphere (real-world scale height varies with temperature but ~8000–8500 m is the commonly cited value for troposphere-scale problems). This is a single-layer isothermal approximation — it doesn't model the actual multi-layer temperature/pressure profile of the real atmosphere, which is a reasonable simplification for a hobby-scale rocket that isn't going particularly high.

## Wind Model

Wind is modeled as a simple **altitude-dependent shear**, but only along `x`:
```
wind_x = basewind + 0.002 * y
wind_z = 0   (crosswind is defined but currently disabled)
```
`basewind` is currently `0`, so wind is purely a function of altitude. Drag is computed relative to this wind vector rather than ground velocity. The `z` crosswind term exists in the code as a placeholder but isn't populated — worth knowing if you're troubleshooting why lateral drift always trends toward one axis.

## Staging

Each stage tracks its own `dry_mass`, `fuel`, `thrust`, and a derived `burn_rate = fuel / burn_time`. Only the **current** stage produces thrust and burns fuel. When a stage's fuel reaches zero:
- If it isn't the last stage, its `dry_mass` is zeroed out (simulating jettison — that mass no longer contributes to `currentmass`) and the simulation advances to the next stage.
- If it *is* the last stage, thrust simply stops and the rocket coasts/falls under gravity and drag alone.

This means total vehicle mass drops discretely at each staging event, which is the correct qualitative behavior for staged rockets (a real rocket's mass also drops in a step at separation, not continuously).

## Pitch Program (Gravity Turn Simplification)

Starting at `time > 2.0 s`, pitch decreases by `0.05°` per timestep:
```
pitch -= 0.05° per 0.01s step  →  5°/s pitch-down rate
```
This is a simplified stand-in for a real gravity turn (which is driven by aerodynamics and guidance, not a fixed schedule). It continues until the final stage burns out, at which point `stoppitch` freezes the pitch angle. Because the rate is fixed and unbounded until that point, a sufficiently long powered phase would in principle pitch the rocket past horizontal (0°) toward negative angles — in practice this is bounded by how long your stages actually burn, but it's worth knowing this isn't a physically-derived guidance law, just a scripted pitch-over.

## Flight Phase Detection (Status Events)

The simulator emits status strings over stdout/WebSocket at specific transitions:

| Status | Condition |
|---|---|
| `Powered Ascent` | First time `vy > 0` after `t > 1s` |
| `Apogee Reached` | See note below |
| `Free Fall` | Final stage's fuel has depleted |
| `Parachute deployed` | `vy < 0` and altitude `≤ 200 m` |
| `Landed` | `y ≤ 0` after `t > 2s` |

**A note on the apogee check worth verifying in your own testing:** the condition is `prevy >= 0 && vy <= 0`, where `prevy` is meant to capture the previous step's vertical velocity so the code can detect the sign flip (positive → negative) at apogee. However, `prevy` is assigned *after* the velocity update at the bottom of the loop, and read again at the top of the *next* loop *before* that iteration's velocity update runs. Since nothing changes `vy` in between those two points, `prevy` and `vy` hold the exact same value at the moment they're compared — the condition effectively becomes "is `vy` simultaneously `≥ 0` and `≤ 0`," which is only true if `vy` lands on exactly `0.0`. At a 10000 Hz timestep with floating-point velocities, that's unlikely to happen naturally, so this message may fire rarely or never in practice. If you've noticed `Apogee Reached` not showing up reliably, this is almost certainly why — the fix would be capturing `prevy` at the *top* of the loop (before the update) rather than the bottom.

## Known Simplifications

- Point-mass rocket — no rotational dynamics, moment of inertia, or torque from off-axis thrust/drag.
- No wind in `z` (crosswind term present but unused).
- Fixed, scripted pitch-over rather than a physically-derived gravity turn.
- No Coriolis or Earth-rotation effects (reasonable at this scale).
- Parachute drag uses a fixed `A*Cd` rather than separate, physically-motivated area and coefficient values.

## Possible Extensions

A few directions that would deepen the physics without a full rewrite:
- Fix the apogee detection and CSV header mismatch noted above.
- Replace the fixed pitch-over with a closed-loop guidance law (e.g., gravity turn driven by angle-of-attack minimization) for a more authentic ascent profile.
- Add rotational dynamics (moment of inertia, angular velocity) so pitch isn't just an animation input but an actual state driven by torques.
- Move from fixed-step semi-implicit Euler to an adaptive-step RK4 integrator for better accuracy during high-dynamics phases (staging, parachute deployment).
