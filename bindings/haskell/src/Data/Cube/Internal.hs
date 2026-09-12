{-# LANGUAGE MultiParamTypeClasses #-}

module Data.Cube.Internal 
(
  Color(..)
, Face(..)
, Move(..)
, ActsOn(..)
, Actionable(..)
, moveFaces
, moveFromRaw
, unsafeMoveFromRaw
, showAction
)
where 

import Data.Cube.Def

import qualified Data.Vector.Sized as V
import Data.Array (Array, listArray, (!))
import Data.List (sort)


instance ActsOn Cube Turn where
    c &> t = c &> toMove t

instance Actionable Turn where 
    toMove t = moveTable_ ! fromEnum t

moveFaces :: Move -> [Face]
moveFaces (Move v) = map toEnum (V.toList v)

showAction :: Actionable a => a -> String
showAction a = "Action " ++ show (moveFaces (toMove a))

moveFromRaw :: [Int] -> Maybe Move
moveFromRaw xs 
    | isValidPerm xs = Move <$> V.fromList (map toEnum xs)
    | otherwise = Nothing
    where 
        isValidPerm ys = sort ys == [0..53]

unsafeMoveFromRaw :: [Int] -> Move
unsafeMoveFromRaw xs = case moveFromRaw xs of 
    Just m -> m
    Nothing -> error "unsafeMoveFromRaw: input is not a valid 0..53 permutation"

moveTable_ :: Array Int Move
moveTable_ = listArray(0,17) (concatMap powers base6) where
    powers v = [v, v <> v, v <> v <> v] 
    base6    = map unsafeMoveFromRaw [mU,mR,mF,mD,mL,mB]
    
    mU = [6,3,0,7,4,1,8,5,2,45,46,47,12,13,14,15,16,17,9,10,11,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,18,19,20,39,40,41,42,43,44,36,37,38,48,49,50,51,52,53]
    mR = [0,1,20,3,4,23,6,7,26,15,12,9,16,13,10,17,14,11,18,19,29,21,22,32,24,25,35,27,28,51,30,31,48,33,34,45,36,37,38,39,40,41,42,43,44,8,46,47,5,49,50,2,52,53]
    mF = [0,1,2,3,4,5,44,41,38,6,10,11,7,13,14,8,16,17,24,21,18,25,22,19,26,23,20,15,12,9,30,31,32,33,34,35,36,37,27,39,40,28,42,43,29,45,46,47,48,49,50,51,52,53]
    mD = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,24,25,26,18,19,20,21,22,23,42,43,44,33,30,27,34,31,28,35,32,29,36,37,38,39,40,41,51,52,53,45,46,47,48,49,50,15,16,17]
    mL = [53,1,2,50,4,5,47,7,8,9,10,11,12,13,14,15,16,17,0,19,20,3,22,23,6,25,26,18,28,29,21,31,32,24,34,35,42,39,36,43,40,37,44,41,38,45,46,33,48,49,30,51,52,27]
    mB = [11,14,17,3,4,5,6,7,8,9,10,35,12,13,34,15,16,33,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,36,39,42,2,37,38,1,40,41,0,43,44,51,48,45,52,49,46,53,50,47]