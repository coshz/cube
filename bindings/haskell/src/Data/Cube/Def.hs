{-# LANGUAGE DataKinds #-}
{-# LANGUAGE MultiParamTypeClasses #-}

module Data.Cube.Def where 

import Data.Cube.Raw (rawBase6)

import Data.Vector.Sized (Vector)
import qualified Data.Vector.Sized as V
import Data.Array (Array, listArray, (!))
import Data.Maybe (fromMaybe)
import Data.List (sort)

data Color = U | R | F | D | L | B
    deriving (Eq, Show, Enum, Bounded)

data Face
    = U1 | U2 | U3 | U4 | U5 | U6 | U7 | U8 | U9
    | R1 | R2 | R3 | R4 | R5 | R6 | R7 | R8 | R9
    | F1 | F2 | F3 | F4 | F5 | F6 | F7 | F8 | F9
    | D1 | D2 | D3 | D4 | D5 | D6 | D7 | D8 | D9
    | L1 | L2 | L3 | L4 | L5 | L6 | L7 | L8 | L9
    | B1 | B2 | B3 | B4 | B5 | B6 | B7 | B8 | B9
    deriving (Eq, Show, Enum, Bounded)

data Turn
    = Ux1 | Ux2 | Ux3
    | Rx1 | Rx2 | Rx3
    | Fx1 | Fx2 | Fx3 
    | Dx1 | Dx2 | Dx3
    | Lx1 | Lx2 | Lx3
    | Bx1 | Bx2 | Bx3
    deriving (Eq, Show, Enum, Bounded)

newtype Cube = Cube (Vector 54 Color) 
    deriving (Eq)

newtype Move = Move (Vector 54 Int) 
    deriving (Eq)

instance Semigroup Move where 
    Move m1 <> Move m2 = Move (V.backpermute m1 m2)

instance Monoid Move where
    mempty = Move (fromMaybe (error "mempty") (V.fromList [0..53]))

class ActsOn c m where 
    (&>) :: c -> m -> c

instance ActsOn Cube Move where
    (Cube c)  &> (Move p) = Cube (V.backpermute c p)  

class Actionable a where 
    toMove :: a -> Move

    toAction :: a -> (Cube -> Cube)
    toAction a = (&> toMove a)

instance Actionable a => Actionable [a] where 
    toMove = mconcat . map toMove

instance Actionable Move where 
    toMove = id

instance Show Cube where
    show (Cube v) = "Cube " ++ show (V.toList v)

instance Show Move where
    show (Move v) = "Move " ++ show (map (toEnum :: Int -> Face) (V.toList v))

instance ActsOn Cube Turn where
    c &> t = c &> toMove t

instance Actionable Turn where 
    toMove t = moveTable_ ! fromEnum t

moveTable_ :: Array Int Move
moveTable_ = listArray(0,17) (concatMap powers base6) where
    powers v = [v, v <> v, v <> v <> v] 
    base6    = map unsafeMoveFromRaw rawBase6

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