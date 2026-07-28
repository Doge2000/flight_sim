import csv
import math
import os
import subprocess


SIM_EXE = os.path.join(os.path.dirname(__file__), "", "sim.exe")
CSVPATH = os.path.join(os.path.dirname(__file__), "", "sim.csv")


g = 9.81
tolerance = 1e-3


#config

thrust = 50
fuel = 1
drymass = 0.5
burntime = 1


def runsim():
    env = os.environ.copy()
    env["SIM_TEST_NODRAG"] = "1"
    result = subprocess.run(
        [SIM_EXE, "1", str(thrust), str(fuel), str(drymass), str(burntime)],
        env = env, capture_output=True, text = True, timeout = 30,
    )

    if result.returncode !=0:
        raise RuntimeError(f"sim.exe failed:\n{result.stderr}")


def rawrows():
    with open(CSVPATH, newline="") as f:
        reader = csv.reader(f)
        next(reader)
        return[[float(x) for x in row] for row in reader]


def analyticalv(t, thrust, m0, burnrate):
    return (thrust/burnrate) * math.log(m0 / (m0 - burnrate*t)) - g*t

def test_poweredascentmatch():
    runsim()
    rows = rawrows()

    m0 = fuel+drymass
    burnrate = fuel/burntime

    checked = 0
    for r in rows:
        t = r[0]
        if t<=0 or t>=burntime:
            continue
        vysim = r[5]
        vyexp = analyticalv(t, thrust, m0, burnrate)
        relerror = abs(vysim-vyexp) / max(abs(vyexp), 1e-9)

        assert relerror < tolerance, (
            f"t={t:.3f}s: sim vy = {vysim:.6f}, expected = {vyexp:.6f}. "
            f"rel error={relerror:.6f}"
        )

        checked+=1
    assert checked > 10, "not enough burn-phase samples were checked --> lower dt or burntime mismatch"



def test_coastphase():
    runsim()
    rows = rawrows()

    burnoutind = next(i for i, r in enumerate(rows) if r[8] <= 0)
    t0, y0, vy0 = rows[burnoutind][0], rows[burnoutind][2], rows[burnoutind][5]


    checked = 0

    for r in rows[burnoutind:]:
        dt = r[0] - t0
        if dt <= 0:
            continue
        ysim = r[2]
        yexp = y0 + vy0 *dt - 0.5*g*dt*dt
        relerr = abs(ysim-yexp) / max(abs(yexp), 1.0)


        assert relerr < 0.005, (
            f"t={r[0]:.3f}s: sim y={ysim:.6f}, expected={yexp:.6f}, "
            f"rel error={relerr:.6f}"
        )
        checked+=1

    assert checked > 10, "not enough coast-phase samples were checked"




    if __name__ == "__main__":
        test_poweredascentmatch()
        print("powered ascent test passed")
        test_coastphase()
        print("coast phase test passed")