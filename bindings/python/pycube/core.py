from ._ffi import _libcube as lib
from ._exceptions import SolveResult, SolveError
import ctypes 

__all__ = [ 'solve', 'facecube', 'permutaion', 'solvable', 'CUBE_ID' ]

CUBE_ID = lib.CUBE_ID

def solve(src: str, tgt: str = CUBE_ID, step: int = 30, best: bool = False) -> str:
    """ Solve the cube.

    Args:
        src     : the source color configuration;
        tgt     : the target color configuration; (None => cid)
        step    : the maximum allowed steps;
        best    : find the shorter (but slower) solution
    
    Return:
        the sequence of moves
    
    Raise:
        SolveError(code) if the src/tgt cube configuration is invalid or 
        the cube is unsolvable or no solution found within the step limit.
    """
    src_bytes = src.encode('utf-8')
    tgt_bytes = tgt.encode('utf-8')
    buffer = ctypes.create_string_buffer(lib.CUBE_BS)
    code = lib.solve_ultimate(src_bytes, tgt_bytes, buffer, step, best, 1)
    if code != SolveResult.SUCCESS: 
        raise SolveError(code)
    return buffer.value.decode('utf-8')


def facecube(manuuver: str, cube: str = CUBE_ID) -> str: 
    maneuver_bytes = manuuver.encode('utf-8')
    cube_bytes = cube.encode('utf-8')
    buffer = ctypes.create_string_buffer(lib.CUBE_BS)
    lib.facecube(cube_bytes,maneuver_bytes,buffer)
    return buffer.value.decode('utf-8')


def permutaion(maneuver: str) -> str: 
    maneuver_bytes = maneuver.encode('utf-8')
    buffer = ctypes.create_string_buffer(lib.CUBE_BS)
    lib.permutation(maneuver_bytes,buffer)
    return buffer.value.decode('utf-8')


def solvable(cube: str) -> bool: 
    cube_bytes = cube.encode('utf-8')
    return lib.solvable(cube_bytes)