# cube-python

[![PyPI version](https://img.shields.io/pypi/v/cube-python.svg)](https://pypi.org/project/cube-python/)
[![PyPI License](https://img.shields.io/pypi/l/cube-python.svg)](https://pypi.org/project/cube-python/)
[![GitHub](https://img.shields.io/badge/github-coshz%2Fcube-blue?logo=github)](https://github.com/coshz/cube/tree/HEAD/bindings/python)


High-performance Rubik's Cube solver Python bindings powered by C/C++ FFI.

---

## Installation

```bash
pip install cube-python
```

## Quickstart

```python
import pycube
from pycube import SolveError

# Default solved cube string representation
print(f"Target State: {pycube.CUBE_ID}")

# Validate state solvability
scrambled = pycube.facecube("U F U' L2 R L' D2 B")

if pycube.solvable(scrambled):
    try:
        # Find solution sequence
        solution = pycube.solve(scrambled,good=22)
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
from pycube import SolveError, CUBE_ID, PermFormat
```

### Functions

| Function | Description |
| :--- | :--- |
| `solve(src,tgt=CUBE_ID,good=22)` | Solve cube state `src`; stops as soon as a solution <= `good` moves is found. Returns solution string or raises `SolveError`. |
| `facecube(maneuver,cube=CUBE_ID)` | Applies move sequence (e.g., `"U F U' L2"`) to a cube state and returns the new state. |
| `solvable(cube)` | Returns `True` if the 54-character state string is physically solvable. |
| `permutation(ms_or_cube,fmt=PermFormat.Cycle)` | Returns a permutation representation for a move sequence or state. |

### Constants & Exceptions

| Name | Type | Description |
| :--- | :--- | :--- |
| `CUBE_ID` | `str` | Constant representing the standard solved state `UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`. |
| `PermFormat` | `IntEnum` | Permutation output format. Members: `Face`,`Cubie`,`Cycle`. Defaults to `Cycle`. |
| `SolveError` | `Exception` | Raised when the state is invalid or unsolvable. |

## 📄 License

[MIT](LICENSE) © coshz
