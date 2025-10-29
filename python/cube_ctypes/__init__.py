from ._lib import _cube_lib, c_solve_ultimate, c_facecube, c_permutation, c_solvable
from .exceptions import CubeError, StatusCode


__all__ = [ 'solve','get_facecube','get_permutaion','is_solvable','CubeError' ]

cid = 'UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB'


def solve_ultimate(src: str, tgt: str = cid, step: int = 30, best: bool = False) -> str:
    """
    Solve the cube
    Args:
        src     : the source color configuration;
        tgt     : the target color configuration; (None => cid)
        step    : the maximum allowed steps;
        best    : find the shorter (but slower) solution
    
    Return:
        the sequence of moves
    
    Raise:
        CubeError(code) if the src/get cube configuration is invalid or 
        the cube is unsolvable or no solution found
    """
    if tgt is None: tgt = cid
    src_bytes = src.encode('utf-8')
    tgt_bytes = tgt.encode('utf-8')
    return c_solve_ultimate(src_bytes, tgt_bytes, step, best)


def solve(src:str, best: bool = False) -> str: 
    return solve_ultimate(src, None, 30, best)


def get_facecube(manuuver: str, cube: str = cid) -> str: 
    maneuver_bytes = manuuver.encode('utf-8')
    cube_bytes = cube.encode('utf-8')
    return c_facecube(cube_bytes,maneuver_bytes)


def get_permutaion(maneuver: str) -> str: 
    maneuver_bytes = maneuver.encode('utf-8')
    return c_permutation(maneuver_bytes)


def is_solvable(cube: str) -> bool: 
    cube_bytes = cube.encode('utf-8')
    return c_solvable(cube_bytes)