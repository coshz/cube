from enum import IntEnum
from ._ffi import _libcube as lib

__all__ = ['SolveResult', 'SolveError']


class SolveResult(IntEnum):
    SUCCESS = 0
    UNSOLVABLE = 1
    NOT_FOUND = 2
    INVALID_SRC = 3
    INVALID_TGT = 4
    UNKNOWN_ERR = 5


class SolveError(Exception):
    def __init__(self, sr: SolveResult):
        self.value = sr
        msg = lib.solve_result_to_string(sr)
        super().__init__(f"[{sr.name}] {msg}")