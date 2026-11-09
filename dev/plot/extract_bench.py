#!/usr/bin/env python3
"""
Emit a single comma-separated line of solve times, one value per benchmark
case, for plotting ECDFs.

- Successful cases emit their actual solve time in ms.
- Timeout/failure cases emit a sentinel ``limit * factor`` (default 1.5 * limit)
  so the ECDF stops at the success ratio once the plot's x-axis is clipped to
  ``limit``.

The timeout limit is auto-detected from the log header
(``Timeout Threshold : 10000 ms``), falling back to ``Exceeded 10000 ms limit``;
override with ``--limit`` only if the log is malformed.

The default output path replaces the leading ``solver`` in the input filename
with ``data`` and keeps the rest of the name unchanged, e.g.
``solver[sym=on]_N100_L30_G22_S1337_T10000_v0051.txt``
  -> ``data[sym=on]_N100_L30_G22_S1337_T10000_v0051.txt``

Usage:
    python extract_benchmark.py "solver[sym=on]_N100_L30_G22_S1337_T10000_v0051.txt"
    python extract_benchmark.py input.txt -o out.txt
    python extract_benchmark.py input.txt --factor 2
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

# Case lines look like:
#   [O]      434 ms  DDDD...
#   [?]       --     UFDB...  [Exceeded 10000 ms limit]
CASE_RE = re.compile(r"^\s*\[(?P<status>[^\]]+)\]\s+(?P<rest>.*)$")
TIME_RE = re.compile(r"(?P<ms>\d+)\s*ms\b")
THRESHOLD_RE = re.compile(r"Timeout\s+Threshold\s*:\s*(?P<ms>\d+)\s*ms", re.IGNORECASE)


def detect_limit(path: Path) -> int | None:
    """Read the timeout limit from the log header"""
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            m = THRESHOLD_RE.search(line)
            if m:
                return int(m.group("ms"))
    return None


def iter_cases(path: Path):
    """Yield (id, status, time_ms_or_None) in file order."""
    idx = -1
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            m = CASE_RE.match(line)
            if not m: continue
            idx += 1
            status = m.group("status").strip()
            t = TIME_RE.search(m.group("rest"))
            time_ms = int(t.group("ms")) if t else None
            yield idx, status, time_ms


def build_series(path: Path, limit: int) -> list[int]:
    """One value per case: real time if [O], else sentinel."""
    sentinel = int(limit*1.5 + 1)
    values: list[int] = []
    for _cid, status, time_ms in iter_cases(path):
        if status == "O" and time_ms is not None:
            values.append(time_ms)
        else:
            values.append(sentinel)
    return values


def format_value(v: float) -> str:
    return str(int(v)) if v.is_integer() else f"{v:g}"


def default_output_path(logfile: Path) -> Path:
    """Replace the leading 'solver' in the filename with 'data', keep the rest."""
    name = logfile.name
    if name.startswith("solver"):
        new_name = "data" + name[len("solver"):]
    else:
        new_name = "data_" + name
    return logfile.with_name(new_name)


def main(argv=None) -> int:
    p = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p.add_argument("logfile", type=Path, help="benchmark file")
    p.add_argument("--outdir", type=Path, help="output directory")
    args = p.parse_args(argv)
    args.outdir = args.outdir or args.logfile.parent

    limit = detect_limit(args.logfile)
    if limit is None: p.error("could not detect timeout limit")

    values = build_series(args.logfile, limit)
    values_line = ",".join(str(v) for v in values)

    if "bench" not in args.logfile.name: 
        p.error(f"file name must contains 'bench': {args.logfile.name}")
    
    datfile = args.outdir / args.logfile.name.replace("bench", "data", 1)
    datfile.write_text(values_line+'\n', encoding="utf-8")
    print(f"{args.logfile.name}: {len(values)} cases -> {datfile}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())