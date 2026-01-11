from pycube import solve, get_facecube, get_permutaion, is_solvable

ms = "U F U' L2 R L' D2 B"

fc = get_facecube(ms)
cs = get_permutaion(ms)

assert(is_solvable(fc))
s = solve(fc, best=True)

print(f"""
    color: {fc}
    permutation: {cs}
    solution: {s}
""")