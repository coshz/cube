from cube_ctypes import solve, get_facecube, get_permutaion, is_solvable

ms = "U L2 F' D B R' L' R2 B' F"

fc = get_facecube(ms)
cs = get_permutaion(ms)
s = solve(fc,True)

print(f"""
    color: {fc}
    permutation: {cs}
    solution: {s}
""")