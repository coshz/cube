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

{-# LANGUAGE FlexibleInstances #-}
{-# LANGUAGE TypeFamilies #-}
{-# LANGUAGE GADTs #-}

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

import Data.Cube.Def
import Data.Cube.FFI
import Data.Cube.Internal ()

import qualified Data.Vector.Sized as V
import Data.Maybe (fromMaybe)
import System.IO.Unsafe (unsafePerformIO)
import Foreign (allocaBytes, Ptr, Word8)
import Foreign.C ( peekCString, CString )
import Foreign.C.String (withCString)
import Foreign.C.Types (CBool(..))
import Foreign.Marshal (peekArray0)
import Foreign.Ptr (castPtr)


cubeId :: Cube
cubeId = Cube (fromMaybe (error "cId") (V.fromList (concatMap (replicate 9) [U .. B])))

showCube :: Cube -> String
showCube (Cube v) = concatMap show (V.toList v)

parseCube :: String -> Maybe Cube
parseCube cubeStr
    | isLegalCubeString cubeStr = do
        colors <- mapM charToColor cubeStr
        Cube <$> V.fromList colors
    | otherwise = Nothing
    where
        charToColor :: Char -> Maybe Color
        charToColor c = case c of
            'U' -> Just U
            'R' -> Just R
            'F' -> Just F
            'D' -> Just D
            'L' -> Just L
            'B' -> Just B
            _ -> Nothing

isLegalCubeString :: String -> Bool
isLegalCubeString str = 
    length str == 54 &&
    unsafePerformIO (withCString str $ \c_str -> do
        CBool res <- c_solvable c_str
        return (res /= 0))

applyTurns :: Cube -> [Turn] -> Cube
applyTurns = foldl (&>)

parseTurns :: String -> Maybe [Turn]
parseTurns str = mapM stringToTurn (words str) where 
    stringToTurn :: String -> Maybe Turn 
    stringToTurn s = case s of 
        "U" -> Just Ux1; "U2" -> Just Ux2; "U'" -> Just Ux3
        "R" -> Just Rx1; "R2" -> Just Rx2; "R'" -> Just Rx3
        "F" -> Just Fx1; "F2" -> Just Fx2; "F'" -> Just Fx3
        "D" -> Just Dx1; "D2" -> Just Dx2; "D'" -> Just Dx3
        "L" -> Just Lx1; "L2" -> Just Lx2; "L'" -> Just Lx3
        "B" -> Just Bx1; "B2" -> Just Bx2; "B'" -> Just Bx3
        _ -> Nothing

showTurns :: [Turn] -> String 
showTurns ts = unwords (map turnToString ts) where 
    turnToString :: Turn -> String 
    turnToString t = case t of 
        Ux1 -> "U"; Ux2 -> "U'"; Ux3 -> "U2"
        Rx1 -> "R"; Rx2 -> "R'"; Rx3 -> "R2"
        Fx1 -> "F"; Fx2 -> "F'"; Fx3 -> "F2"
        Dx1 -> "D"; Dx2 -> "D'"; Dx3 -> "D2"
        Lx1 -> "L"; Lx2 -> "L'"; Lx3 -> "L2"
        Bx1 -> "B"; Bx2 -> "B'"; Bx3 -> "B2"

solve :: Cube -> Cube -> Either String [Turn]
solve src tgt = unsafePerformIO $
    withCString (showCube src) $ \c_src ->
    withCString (showCube tgt) $ \c_tgt ->
    allocaBytes 128 $ \c_buf -> do
        resCode <- c_solve_ultimate c_src c_tgt c_buf 30 (CBool 1) 0
        if resCode == 0
            then parseCSolution c_buf
            else do
                errPtr <- c_solve_result_to_string resCode
                errStr <- peekCString errPtr
                return (Left $ "Solver error (" ++ show resCode ++ "): "++ errStr)
    where
        parseCSolution :: CString -> IO (Either String [Turn])
        parseCSolution buf = do
            bytes <- peekArray0 0 (castPtr buf :: Ptr Word8)
            return (mapM byteToTurn bytes)

        byteToTurn :: Word8 -> Either String Turn
        byteToTurn b
            | b >= 1 && b <= 18 = Right (toEnum (fromIntegral b -1))
            | otherwise = Left $ "Invalid byte from solver: " ++ show b

unsafeSolve :: Cube -> Cube -> [Turn]
unsafeSolve src tgt = case solve src tgt of 
    Left err -> error err
    Right ts -> ts

solveFrom :: Cube -> Either String [Turn]
solveFrom c = solve c cubeId 

unsafeSolveFrom :: Cube -> [Turn]
unsafeSolveFrom c = unsafeSolve c cubeId