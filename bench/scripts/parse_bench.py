from __future__ import annotations

import re
import os

CASE_RE = re.compile(r"^\s*\[(?P<status>[^\]]+)\]\s+(?P<rest>.*)$")
TIME_RE = re.compile(r"(?P<ms>\d+)\s*ms\b")
THRESHOLD_RE = re.compile(r"Timeout\s+Threshold\s*:\s*(?P<ms>\d+)\s*ms", re.IGNORECASE)


def parse_benchmark(path: str | os.PathLike[str]) -> tuple[int, list[int]]:
    """Return (timeout_limit_ms, per-case times_ms).

    Successful ``[O]`` cases keep their measured time; every other case gets a
    sentinel of ``1.5 * limit + 1`` so an ECDF clipped at ``limit`` stops at
    the success ratio.
    """
    limit: int | None = None
    raw: list[int | None] = []

    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            if limit is None:
                m = THRESHOLD_RE.search(line)
                if m:
                    limit = int(m.group("ms"))
                    continue
            m = CASE_RE.match(line)
            if m is None:
                continue
            status = m.group("status").strip()
            t = TIME_RE.search(m.group("rest"))
            ms = int(t.group("ms")) if t else None
            raw.append(ms if status == "O" and ms is not None else None)

    if limit is None:
        raise ValueError(f"no 'Timeout Threshold' header in {path}")

    sentinel = limit * 3 // 2 + 1
    return limit, [v if v is not None else sentinel for v in raw]


def parse_bench_name(filename: str) -> dict:
    """
    Parse version, symmetry and parameters from benchmark filename.

    Args: e.g.
        v0051-bench_SYMT_N100_L30_G22_S1337_T10000
        v0051-bench_SYMF_N100_L30_G22_S1337_T10000

    Return: 
        A dict with keys ["ver", "sym", "N", "L", "G", "S", "T"]
    """
    ver_match = re.search(r'v(\d+)', filename)
    if not ver_match: raise ValueError(f"Malformed name: {filename}")
    ver: str = ver_match.group(1)

    sym_match = re.search(r'SYM(T|F)', filename)
    if not sym_match: raise ValueError(f"Malformed name: {filename}")
    sym: bool = sym_match.group(1) == "T"

    params = { k: int(v) for k,v in re.findall(r'_([NLGST])(\d+)', filename) }

    return {
        "ver": ver,
        "sym": sym,
        "N": params.get('N'),
        "L": params.get('L'),
        "G": params.get('G'),
        "S": params.get('S'), 
        "T": params.get('T')
    }