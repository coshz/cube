-- |
-- Module      : Data.Cube
-- Description : High-performance, type-safe Rubik's Cube library
-- Copyright   : (c) coshz, 2026
-- License     : MIT
-- Maintainer  : fsinhx@gmail.com
-- Stability   : stable
--
-- This module provides a safe-by-default Haskell interface for 3x3 Rubik's Cube 
-- manipulation and solving via C FFI.
--
-- === Example Usage
-- @
-- import Data.Cube
--
-- main :: IO ()
-- main = do
--     -- 1. Verify standard solved state representation
--     let initStr = showCube cubeId
--     putStrLn $ "Is initial cube legal? " ++ show (isLegalCubeString initStr)
--
--     -- 2. Parse standard WCA scramble sequence
--     let scrambledTurns = fromMaybe [] $ parseTurns "U F U' L2 R L' D2 B"
--     putStrLn $ "Is the count of turns is 8? " ++ show (length scrambledTurns == 8)
--
--     -- 3. Apply group action (&>) to scramble the solved identity cube
--     let scrambledCube = foldl (&>) cubeId scrambledTurns
--     putStrLn $ "Scrambled state: " ++ showCube scrambledCube
--
--     -- 4. Solve using zero-cost C FFI solver and verify restoration
--     let sol = unsafeSolveFrom scrambledCube
--     putStrLn $ "Optimal solution steps: " ++ showTurns sol
--     putStrLn $ "Is fully restored? " ++ show (applyTurns scrambledCube sol == cubeId)
-- @

module Data.Cube
( 
  -- * Core Types
  Turn(..)
, Cube
  -- * Identity & Constants
, cubeId
  -- * Validation & Formatting
, isLegalCubeString
, showCube
, parseCube
  -- * Algebraic Operations & Group Actions 
, (&>)
, applyTurns
  -- * WCA Turns Representation
, showTurns
, parseTurns
  -- * Solving Interface (C FFI)
, solve
, unsafeSolve
, solveFrom
, unsafeSolveFrom
) where

import Data.Cube.Internal