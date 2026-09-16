# cube-hs

[![Hackage](https://img.shields.io/hackage/v/cube-hs.svg)](https://hackage.haskell.org/package/cube-hs)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![GitHub](https://img.shields.io/badge/github-coshz%2Fcube-blue?logo=github)](https://github.com/coshz/cube/tree/master/bindings/haskell)

A performant and type-safe Rubik's Cube manipulation and permutation library for Haskell.

## Features

- **Type-Safe Representation**: Strongly typed representations of Cubes, Faces, Turns, and Moves.
- **Algebraic Group Actions**: Pure permutation math leveraging `Semigroup`, `Monoid`, and the `ActsOn` typeclass.
- **Flexible Interactions**: Turn individual faces, apply sequence moves, or compose transformations seamlessly.

## Installation

Add `cube-hs` to your project's `.cabal` file dependencies:

```cabal
build-depends: base >= 4.14 && < 5, cube-hs
```

Or install it via `cabal`: 

```bash
cabal update
cabal install cube-hs
```

## Usage

```haskell 
module Main where

import Data.Cube
import Data.Maybe (fromMaybe)

main :: IO ()
main = do
    -- 1. Inspect solved cube state
    let solvedCube = cubeId
    putStrLn $ "Solved Cube Colors: " ++ showCube solvedCube
    -- "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"

    -- 2. Parse standard turn notations
    let turnNotation = "U F U' L2 R L' D2 B"
    let turns = fromMaybe [] $ parseTurns turnNotation
    
    -- 3. Scramble the cube by folding turns
    let scrambledCube = foldl (&>) solvedCube turns
    putStrLn $ "Scrambled Cube:     " ++ showCube scrambledCube

    -- 4. Solve the scrambled cube & verify
    let solution = unsafeSolveFrom scrambledCube
    let restoredCube = applyTurns scrambledCube solution

    if restoredCube == solvedCube
        then putStrLn $ "Solution: " ++ showTurns solution ++ "."
        else putStrLn "Solving failed."
```

