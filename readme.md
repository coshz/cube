## Introduction

This project is a Rubik's Cube solver built around a C++ re-implementation 
of Kociemba's twophase algorithm, inspired by the original mathematica code from [1].

It inclues:
  - libcube: the core C++ library with a C-style interface
  - Python binding via [ctypes](https://docs.python.org/3/library/ctypes.html)
  - JavaScript binding via [emscripten](https://emscripten.org/)
  - Command-line interface (icube) for interactive solving

CLI Usage (icube)

```txt
$ ./icube
Welcome! This is a Rubik's cube solver.
(*`:h` for help, `:q` for quit *)

In [0] := :h
[Help]
    solve <src> [tgt=cid] [best=1] [N=30]  -- find [best] solution form <src> to [tgt] within [N] steps
    color <maneuver> [cube=cid]            -- color by applying maneuver to cube
    perm  <maneuver>                       -- decompose maneuver to cubies permutation

where:
   <...>           -- required argument
   [...]           -- optional argument
   src,tgt,cube    :: the color configuration; eg: `UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`
   maneuver        :: the move sequence;       eg: `FRL'B2D`, `(DR'F2L){7} BD2`

In [0] := color "U F U' L2 R L' D2 B"

Out[0] => FFLBURBURUUFRRRDDBUFBUFDFBRUDFBDFLLRDLLULLBRRULLFBBDDD

In [1] := solve "FFLBURBURUUFRRRDDBUFBUFDFBRUDFBDFLLRDLLULLBRRULLFBBDDD"

Out[1] => B' D2 R' L' U F' U'
```

## References

1. [http://kociemba.org/cube.htm](http://kociemba.org/cube.htm)


