"""
Python ctypes wrapper and FFI definitions for the C++ shared library.

Ref: 
    ../cxx_src/cube.h
    
See Also:
    ../cxx_src/cube_amalg.min.cpp
"""

from ctypes import CDLL, c_char_p, c_int32, c_bool
from pathlib import Path

class CubeCDLL(CDLL):
    CUBE_BS = 128
    CUBE_ID = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"


def _load_library() -> CubeCDLL:
    package_dir = Path(__file__).parent 
    allowed_suffixes = { '.so', '.dylib', '.dll', '.pyd' }
    candidates = [ 
        p for p in package_dir.glob("_native*")
        if p.suffix in allowed_suffixes
    ]

    if not candidates: 
        raise FileNotFoundError(
            f"Could not find compiled C++ shared library inside '{package_dir}'.\n"
            "If you are developing locally, run `pip install -e .` or "
            "`python setup.py build_ext --inplace` to build the native extension first."
        )
        
    lib_path = str(candidates[0])
    return CubeCDLL(lib_path)


_libcube = _load_library() 

_libcube.solve.argtypes = [c_char_p, c_char_p,  c_char_p, c_int32, c_bool]
_libcube.solve.restype = c_int32

_libcube.facecube.argtypes = [c_char_p, c_char_p, c_char_p]
_libcube.facecube.restype = None

_libcube.solvable.argtypes = [c_char_p]
_libcube.solvable.restype = c_bool

_libcube.permutation.argtypes = [c_char_p, c_char_p]
_libcube.permutation.restype = None

_libcube.solve_result_to_string.argtypes = [c_int32]
_libcube.solve_result_to_string.restype = c_char_p
