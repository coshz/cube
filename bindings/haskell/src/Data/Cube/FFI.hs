module Data.Cube.FFI where

import Foreign.C.Types (CInt(..), CBool(..))
import Foreign.C.String (CString)

foreign import ccall "solve_ultimate"
    c_solve_ultimate 
        :: CString  -- src
        -> CString  -- tgt
        -> CString  -- solution_buffer
        -> CInt     -- step
        -> CBool    -- best
        -> CInt     -- formated
        -> IO CInt

foreign import ccall "solvable"
    c_solvable :: CString -> IO CBool

foreign import ccall "solve_result_to_string"
    c_solve_result_to_string :: CInt -> IO CString