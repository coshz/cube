#!/usr/bin/env python3
"""
Sync C++ amalgamation source and header files into bindings/python/cxx_src/
"""

import shutil
import sys
from pathlib import Path

def sync():
    script_dir = Path(__file__).resolve().parent
    python_dir = script_dir.parent      
    repo_root = script_dir.parents[2]   
    target_dir = python_dir / "cxx"

    cxx_files = [
        "amalg/cube_amalg.min.cpp",
        "include/cube/cube.h",
    ]

    abs_files = [repo_root / f for f in cxx_files]
    missing_files = [ p for p in abs_files if not p.exists()]
    
    if missing_files: 
        formatted = "\n".join(f"  -{p}" for p in missing_files)
        print(f"[sync_cxx] ERROR: Cannot find the following files:\n{formatted}", file=sys.stderr)
        sys.exit(1)
    
    target_dir.mkdir(parents=True, exist_ok=True)
    for src in abs_files:
        dst = target_dir / src.name
        if not dst.exists() or src.stat().st_mtime > dst.stat().st_mtime:
            shutil.copy2(src, dst)
            print(f"[sync_cxx] Updated: {src.name} -> cxx_src/{src.name}")
        else:
            print(f"[sync_cxx] Up-to-date: cxx_src/{src.name}")


if __name__ == "__main__":
    sync()