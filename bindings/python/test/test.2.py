import pycube
from pycube import SolveError

# Default solved cube string representation
print(f"Target State: {pycube.CUBE_ID}")

# Validate state solvability
scrambled = pycube.facecube("U F U' L2 R L' D2 B")

if pycube.solvable(scrambled):
    try:
        # Find solution sequence
        solution = pycube.solve(scrambled, step=30, best=True)
        print(f"Solution: {solution}")

        # Apply solution to verify end state
        end_state = pycube.facecube(solution, scrambled)
        assert end_state == pycube.CUBE_ID

    except SolveError as e:
        print(f"Failed to solve cube: {e}")
