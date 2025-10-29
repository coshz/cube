import ctypes 
import os
import platform 


__all__ = [ 'c_solve_ultimate', 'c_facecube', 'c_permutation', 'c_solvable' ]


CUBE_BS = 128
CUBE_ID = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
DEFAULT_STEP=30

from .exceptions import check_status 


def _load_library(): 
    def default_lib_path(): 
        # the first available library in current directory
        curr_dir = os.path.dirname(__file__)
        if platform.system() == "Windows": 
            libs = [f for f in os.listdir(curr_dir) if f.endswith(".dll")]
        elif platform.system() == "Darwin": 
            libs = [f for f in os.listdir(curr_dir) if f.endswith(".dylib")]
        else: 
            libs = [f for f in os.listdir(curr_dir) if f.endswith(".so")]
        if libs:
            return os.path.join(curr_dir,libs[0])
        else:
            return None
    lib_path = default_lib_path()
    if not lib_path: 
        lib_path_file = os.path.join(os.path.dirname(__file__),"_libpath")
        with open(lib_path_file,'r') as f:
            lib_path = f.read().split('=')[-1].strip(' ').strip('"') 
            if not os.path.isabs(lib_path):
                lib_path = os.path.join(os.path.dirname(__file__),lib_path)
            lib_path = os.path.expanduser(lib_path)
            if not os.path.exists(lib_path):
                raise FileNotFoundError(f"`{lib_path}` not found")
    try:  
        return ctypes.CDLL(lib_path)
    except OSError as e:
        raise ImportError(f"Failed to load C library {os.path.basename(lib_path)}: {e}" )

_cube_lib = _load_library() 

_cube_lib.solve_ultimate.argtypes = [ctypes.c_char_p, ctypes.c_char_p,  ctypes.c_char_p, ctypes.c_int, ctypes.c_int]
_cube_lib.solve_ultimate.restype = ctypes.c_int

_cube_lib.facecube.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
_cube_lib.facecube.restype = None

_cube_lib.solvable.argtypes = [ctypes.c_char_p]
_cube_lib.solvable.restype = ctypes.c_int

_cube_lib.permutation.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
_cube_lib.permutation.restype = None


def c_solve_ultimate(src_bytes, tgt_bytes, step, best):
    buffer = ctypes.create_string_buffer(CUBE_BS)
    result_code = _cube_lib.solve_ultimate(
        src_bytes,
        tgt_bytes,
        buffer,
        step,
        best
    )
    check_status(result_code) 
    return buffer.value.decode('utf-8')


def c_facecube(cube_bytes, maneuver_bytes):
    buffer = ctypes.create_string_buffer(CUBE_BS)
    _cube_lib.facecube(cube_bytes, maneuver_bytes, buffer)
    return buffer.value.decode('utf-8')


def c_permutation(maneuver_bytes):
    buffer = ctypes.create_string_buffer(CUBE_BS)
    _cube_lib.permutation(maneuver_bytes, buffer)
    return buffer.value.decode('utf-8')


def c_solvable(cube_bytes):
    return _cube_lib.solvable(cube_bytes)