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
, cubeInit
, rawSolve
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

-- | The solved state of a Rubik's cube.
cubeId :: Cube
cubeId = Cube (fromMaybe (error "cId") (V.fromList (concatMap (replicate 9) [U .. B])))

-- | Converts a cube into its 54-character string representation.
showCube :: Cube -> String
showCube (Cube v) = concatMap show (V.toList v)

-- | Parses a 54-character string into a `Cube`, returning `Nothing` if invalid.
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

-- | Checks if a 54-character string represents a legal (solvable) cube state.
isLegalCubeString :: String -> Bool
isLegalCubeString str = 
    length str == 54 &&
    unsafePerformIO (withCString str $ \c_str -> do
        CBool res <- c_solvable c_str
        return (res /= 0))

-- | Applies a sequence of turns to a cube.
applyTurns :: Cube -> [Turn] -> Cube
applyTurns = foldl (&>)

-- | Parses a space-separated string of turns (e.g., "R U R' U'") into a list of `Turn`.
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

-- | Converts a list of turns into a space-separated string representation.
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

-- | Initialize the cube library by loading or building lookup tables.
--
-- @useSym@: 'False' uses compact tables (<10 MB, fast build, slower
-- solving); 'True' uses symmetry tables (~260 MB, slow build, fasters olving).
--
-- @dir@: directory to read\/write table files; empty means the default.
-- Tables are loaded if present, otherwise built there.
--
-- Only the first call takes effect; later calls are no-ops. The first 
-- 'c_solve' invokes it implicitly with defaults.
--
-- Warning: @useSym = True@ may take minutes and write ~260 MB on first run.
cubeInit :: Bool -> FilePath -> IO ()
cubeInit useSymmetry tableDir = do
    let cb = if useSymmetry then 1 else 0
    withCString tableDir $ \cstr -> do
        c_cube_init cb cstr

-- | Uses Kociemba's two-phase algorithm. The search stops as soon as any solution 
-- of length at most `good` is found, trading optimality for speed. 
-- If the returned maneuver is *longer* than `good`, no shorter solution exists
-- and the result is the two-phase optimum. 
rawSolve :: Cube -> Cube -> Int -> Either String [Turn]
rawSolve src tgt good = unsafePerformIO $
    withCString (showCube src) $ \c_src ->
    withCString (showCube tgt) $ \c_tgt ->
    allocaBytes 128 $ \c_buf -> do
        resCode <- c_solve c_buf c_src c_tgt (fromIntegral good)
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

-- | Solves the cube using a default search depth limit of 22.
-- For fine-grained control over the search depth, use `rawSolve`.
solve :: Cube -> Cube -> Either String [Turn]
solve src tgt = rawSolve src tgt 22

-- | Same as `solve`, but throws a runtime error if no solution is found 
-- or if parsing fails. Use this only when Cube is known to be legal.
unsafeSolve :: Cube -> Cube -> [Turn]
unsafeSolve src tgt = case solve src tgt of 
    Left err -> error err
    Right ts -> ts

-- | Solves the given cube back to the solved state (`cubeId`).
solveFrom :: Cube -> Either String [Turn]
solveFrom c = solve c cubeId 

-- | Same as `solveFrom`, but throws a runtime error on failure.
unsafeSolveFrom :: Cube -> [Turn]
unsafeSolveFrom c = unsafeSolve c cubeId