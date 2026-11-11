module Data.Cube.FFI where

import Foreign.C.Types (CInt(..), CBool(..))
import Foreign.C.String (CString)

foreign import ccall "cube_init"
    c_cube_init
        :: CBool    -- use symmetry
        -> CString  -- table directory
        -> IO ()

foreign import ccall "solve"
    c_solve
        :: CString  -- buffer
        -> CString  -- src color
        -> CString  -- tgt color
        -> CInt     -- good enough
        -> IO CInt

foreign import ccall "solvable"
    c_solvable :: CString -> IO CBool

foreign import ccall "permutation"
    c_permutation
        :: CString  -- buffer
        -> CString  -- cube or maneuver string
        -> IO CInt

foreign import ccall "solve_result_to_string"
    c_solve_result_to_string :: CInt -> IO CString
