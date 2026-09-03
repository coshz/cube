import ctypes 
import os
import platform 


class CubeCDLL(ctypes.CDLL):
    CUBE_BS = 128
    CUBE_ID = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"


def _load_library() -> CubeCDLL:
    if platform.system() == "Windows":
        lib_name = "cube.dll"
    elif platform.system() == "Darwin":
        lib_name = "libcube.dylib"
    else:
        lib_name = "libcube.so"

    lib_path = os.path.join(
        os.path.dirname(__file__), lib_name
    )

    if not os.path.exists(lib_path):
        lib_path = lib_name 

    return CubeCDLL(lib_path)

_libcube = _load_library() 

_libcube.solve_ultimate.argtypes = [ctypes.c_char_p, ctypes.c_char_p,  ctypes.c_char_p, ctypes.c_int, ctypes.c_bool, ctypes.c_int]
_libcube.solve_ultimate.restype = ctypes.c_int

_libcube.facecube.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
_libcube.facecube.restype = None

_libcube.solvable.argtypes = [ctypes.c_char_p]
_libcube.solvable.restype = ctypes.c_bool

_libcube.permutation.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
_libcube.permutation.restype = None

_libcube.solve_result_to_string.argtypes = [ctypes.c_int32]
_libcube.solve_result_to_string.restype = ctypes.c_char_p