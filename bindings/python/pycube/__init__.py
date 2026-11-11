from ._ffi import _libcube as lib
from ._exceptions import SolveResult, SolveError
import ctypes
from enum import IntEnum

__all__ = [ 'solve', 'facecube', 'permutation', 'solvable', 'CUBE_ID', 'PermFormat' ]

CUBE_ID = lib.CUBE_ID

class PermFormat(IntEnum):
    Face    = 0
    Cubie   = 1
    Cycle   = 2

def cube_init(use_symmetry: bool = True, table_dir: str = ""):
    """
    Initialize the cube library by loading or building lookup tables.
    Args:
        use_symmetry: ``False`` uses compact tables (<10 MB, fast build,
            slower solving); ``True`` uses symmetry tables (~260 MB, slow
            build, faster solving).
        table_dir: Directory to read/write table files; empty string means
            the default. Tables are loaded if present, otherwise built there.
    Note:
        Only the first call takes effect; later calls are no-ops and their
        arguments are ignored. The first :func:`solve` invokes it implicitly
        with defaults.
    Warning:
        With ``use_symmetry=True``, the first run may take minutes and write
        ~260 MB to ``table_dir``.
    """
    dir = table_dir.encode('utf-8')
    lib.cube_init(use_symmetry, dir)


def solve(src: str, tgt: str = CUBE_ID, good: int = 22) -> str:
    """ Solve the cube.

    Args:
        src     : the source color configuration;
        tgt     : the target color configuration; (None => cid)
        good    : the acceptable steps;

    Return:
        the sequence of moves

    Raise:
        SolveError(code) if the src/tgt cube configuration is invalid or
        the cube is unsolvable or no solution found within the step limit.
    """
    src_bytes = src.encode('utf-8')
    tgt_bytes = tgt.encode('utf-8')
    buffer = ctypes.create_string_buffer(lib.CUBE_BS)
    code = lib.solve(buffer,src_bytes,tgt_bytes,good)
    if code != SolveResult.SUCCESS:
        raise SolveError(code)
    return buffer.value.decode('utf-8')


def facecube(manuuver: str, cube: str = CUBE_ID) -> str:
    maneuver_bytes = manuuver.encode('utf-8')
    cube_bytes = cube.encode('utf-8')
    buffer = ctypes.create_string_buffer(lib.CUBE_BS)
    lib.facecube(buffer,maneuver_bytes,cube_bytes)
    return buffer.value.decode('utf-8')


def permutation(ms_or_cube: str, fmt: PermFormat = PermFormat.Cycle) -> str:
    mc = ms_or_cube.encode('utf-8')
    buffer = ctypes.create_string_buffer(lib.CUBE_BS)
    lib.permutation(buffer,mc,fmt)
    return buffer.value.decode('utf-8')


def solvable(cube: str) -> bool:
    cube_bytes = cube.encode('utf-8')
    return lib.solvable(cube_bytes)
