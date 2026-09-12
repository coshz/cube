module Main (main) where

import Data.Cube

import System.Exit (exitFailure)
import Data.Maybe (fromMaybe)

assertTest :: String -> Bool -> IO() 
assertTest name predicate = do 
    putStr $ "[Test] -- " ++ name ++ "..."
    if predicate 
        then putStrLn "\ESC[32m[PASSED]\ESC[0m"
        else do
            putStrLn "\ESC[31m[FAILED]\ESC[0m"
            exitFailure


main :: IO ()
main = do 
    let initStr = showCube cubeId 
    assertTest "Initial cube color string matches standard" 
                (initStr == "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB")

    assertTest "Standard initial cube is legal" 
                (isLegalCubeString initStr)
    
    let identityMoves = unsafeSolveFrom cubeId
    assertTest "Solving already solved cube returns no moves" 
                (null identityMoves)

    let scrambledTurns = fromMaybe [] $ parseTurns "U F U' L2 R L' D2 B"
    assertTest "Parsing valid turns"
                (scrambledTurns == [Ux1, Fx1, Ux3, Lx2, Rx1, Lx3, Dx2, Bx1])
    
    let scrambledCube = foldl (&>) cubeId scrambledTurns
    assertTest "Verifying scrambled cube string"
            (showCube scrambledCube == "FFLBURBURUUFRRRDDBUFBUFDFBRUDFBDFLLRDLLULLBRRULLFBBDDD")

    let sol = unsafeSolveFrom scrambledCube
    assertTest "Verifying solution"
                (applyTurns scrambledCube sol == cubeId)
