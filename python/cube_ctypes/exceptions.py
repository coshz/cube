
from enum import IntEnum


class StatusCode(IntEnum):
    OK = 0
    UNSOLVABLE = 1
    NOT_FOUND = 2
    INVALID_SRC = 3
    INVALID_TGT = 4
    UNKNOWN_ERROR = 5


class CubeError(Exception):
    def __init__(self,code,message):
        self.code = code 
        super().__init__(f"[{code}] {message}")


def check_status(result_code: int):
    code = StatusCode(result_code)
    if code == StatusCode.OK: return 
    if code == StatusCode.INVALID_SRC:
        msg = "The source cube configuration is invalid."
    elif code == StatusCode.INVALID_TGT:
        msg = "The target cube configuration is invalid."
    elif code == StatusCode.UNSOLVABLE:
        msg = "The cube configuration is unsolvable."
    elif code == StatusCode.NOT_FOUND:
        msg = "No solution found within the step limit."
    else:
        msg = "Unknown error occurred."
    raise CubeError(result_code, msg)