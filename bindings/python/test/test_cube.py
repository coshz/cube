import sys
import os

pycube_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(pycube_dir)

from pycube import solve, facecube, permutaion, solvable

ms = "U F U' L2 R L' D2 B"
fc = facecube(ms)
cs = permutaion(ms)

assert(solvable(fc))
s = solve(fc, best=True)

print(f"""
color: {fc}
permutation: {cs}
solution: {s}
""")