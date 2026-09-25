from enum import IntEnum

__all__ = ['SolveResult', 'SolveError']


class SolveResult(IntEnum):
    SUCCESS = 0
    UNSOLVABLE = 1
    NOT_FOUND = 2
    INVALID_SRC = 3
    INVALID_TGT = 4
    UNKNOWN_ERR = 5


class SolveError(Exception):
    _MSG_MAP = {
        SolveResult.SUCCESS:        "Success",
        SolveResult.UNSOLVABLE:     "The cube configuration is unsolvable",
        SolveResult.NOT_FOUND:      "No solution found within the step limit",
        SolveResult.INVALID_SRC:    "Invalid source color configuration",
        SolveResult.INVALID_TGT:    "Invalid target color configuration",
        SolveResult.UNKNOWN_ERR:    "Unknown error",
    }

    def __init__(self, sr: SolveResult):
        self.value = sr
        super().__init__(str(self))

    def __str__(self):
        msg = self._MSG_MAP.get(self.value, "Unknown error")
        return f"[{self.value.name}] {msg}"