# cube-python

[![PyPI version](https://img.shields.io/pypi/v/cube-python.svg)](https://pypi.org/project/cube-python/)
[![PyPI License](https://img.shields.io/pypi/l/cube-python.svg)](https://pypi.org/project/cube-python/)
[![GitHub](https://img.shields.io/badge/github-coshz%2Fcube-blue?logo=github)](https://github.com/coshz/cube/tree/master/bindings/python)


High-performance Rubik's Cube solver Python bindings powered by C/C++ FFI.

---

## Installation

```bash
pip install cube-python
```

## Quickstart

```python
iimport pycube
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
```

## 📖 API Reference

```python
import pycube
from pycube import SolveError, CUBE_ID
```

### Functions

| Function | Description |
| :--- | :--- |
| `solve(src, tgt=CUBE_ID, step=30, best=False)` | Solves the cube state string `src`. Returns solution string or raises `SolveError`. |
| `facecube(maneuver, cube=CUBE_ID)` | Applies move sequence (e.g., `"U F U' L2"`) to a cube state and returns the new state. |
| `solvable(cube)` | Returns `True` if the 54-character state string is physically solvable. |
| `permutation(ms, fmt=2)` | Returns a permutation representation for a move sequence or state. |

### Constants & Exceptions

| Name | Type | Description |
| :--- | :--- | :--- |
| `CUBE_ID` | `str` | Constant representing the standard solved state (`"UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"`). |
| `SolveError` | `Exception` | Raised when the state is invalid, unsolvable, or exceeds search step limits. |

## 📄 License

[MIT](LICENSE) © coshz
