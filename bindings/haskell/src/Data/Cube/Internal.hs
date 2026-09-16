{-# LANGUAGE MultiParamTypeClasses #-}
{-# LANGUAGE FlexibleInstances #-}
{-# LANGUAGE TypeFamilies #-}
{-# LANGUAGE GADTs #-}

module Data.Cube.Internal 
(
  Color(..)
, Face(..)
, Turn(..)
, Move(..)
, Cube
, ActsOn(..)
, Actionable(..)
, moveFaces
, moveFromRaw
, unsafeMoveFromRaw
, showAction
, cubeId
, isLegalCubeString
, showCube
, parseCube
, applyTurns
, showTurns
, parseTurns
, solve
, unsafeSolve
, solveFrom
, unsafeSolveFrom
)
where 

import Data.Cube.Def
import Data.Cube.FFI

import qualified Data.Vector.Sized as V
import Data.Maybe (fromMaybe)
import System.IO.Unsafe (unsafePerformIO)
import Foreign (allocaBytes)
import Foreign.C (peekCString)
import Foreign.C.String (withCString)
import Foreign.C.Types (CBool(..))


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
        Ux1 -> "U"; Ux2 -> "U2"; Ux3 -> "U'"
        Rx1 -> "R"; Rx2 -> "R2"; Rx3 -> "R'"
        Fx1 -> "F"; Fx2 -> "F2"; Fx3 -> "F'"
        Dx1 -> "D"; Dx2 -> "D2"; Dx3 -> "D'"
        Lx1 -> "L"; Lx2 -> "L2"; Lx3 -> "L'"
        Bx1 -> "B"; Bx2 -> "B2"; Bx3 -> "B'"

solve :: Cube -> Cube -> Either String [Turn]
solve src tgt = unsafePerformIO $
    withCString (showCube src) $ \c_src ->
    withCString (showCube tgt) $ \c_tgt ->
    allocaBytes 128 $ \c_buf -> do
        resCode <- c_solve c_buf c_src c_tgt 30 (CBool 1)
        if resCode == 0
            then parseCSolution c_buf
            else handleErr resCode  
    where
        parseCSolution buf = do 
            str <- peekCString buf
            return $ case parseTurns str of
                Just turns  -> Right turns
                Nothing     -> Left $ "failed to parse solver string: " ++ show str
    
        handleErr code = do 
            errPtr <- c_solve_result_to_string code
            errStr <- peekCString errPtr
            return $ Left $ "Solver error (" ++ show code ++ "): "++ errStr


unsafeSolve :: Cube -> Cube -> [Turn]
unsafeSolve src tgt = case solve src tgt of 
    Left err -> error err
    Right ts -> ts

solveFrom :: Cube -> Either String [Turn]
solveFrom c = solve c cubeId 

unsafeSolveFrom :: Cube -> [Turn]
unsafeSolveFrom c = unsafeSolve c cubeId