#pragma once

#include <cube/cube_export.h> // CUBE_EXPORT
#include <cube/version.h>

#include <stdbool.h>
#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
    #define CUBE_EXPORT_FORCE CUBE_EXPORT __attribute__((used))
#else 
    #define CUBE_EXPORT_FORCE CUBE_EXPORT
#endif

/*******************************************************************************
 @remark
 Items in the cube are defined as follows:

    color       :: { U,R,F,D,L,B } 
    move        :: { U,U2,U',R,R2,R',F,F2,F',D,D2,D',L,L2,L',B,B2,B' } (1~18)
    cubie       :: corner (ufl,ubr,...) OR edge (uf,ub,...) OR center (u,r,f,d,l,b)
    color_cube  :: the color configuration of cube, eg: 
                   cid = `UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`
    maneuver    :: the move sequence, eg: `(DR'F2L){7} BD2`
    permutation :: the cycle-decomposited permutation of cubies, eg: 
                   `(ufl,urf,ubr)(uf,ul,ur)(+u)(−d)`

    a color configuration is:
        solvable if: it could be obtained by moves onto the identity cube;
        valid if: it is solvable up to edge flips or corner twists.

*******************************************************************************/

/*!
 * @remark 
 *  The size of buffer to store: 
 *   - color cube    (* len=54 *);
 *   - solution      (* len<=30 OR len<=90 formated *);
 *   - permutation   (* len < 4 * L"(+urf,+ulb)" + 6 * L"(+ur,+ul)" + 6 * L"(++u)" = 4*11+6*9+6*5 = 128 *)
 * !!! the buffer size should be at least 128!!! 
 */
#define CUBE_BS 128

/* the identity of color configuration */
#define CUBE_ID "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"

#ifndef CF_ENUM
    #if defined(__cplusplus)
        #define CF_ENUM(_type, _name) \
            int __CF_ENUM_##_name; \
            enum _name : _type
    #elif defined(__clang__) || defined(__OBJC__)
        #define CF_ENUM(_type, _name) \
            enum _name : _type _name; \
            enum _name: _type
    #else 
        #define CF_ENUM(_type, _name) \
            _type _name; \
            enum
    #endif
#endif

typedef CF_ENUM(int32_t,SolveResult) {
    SolveResultSuccess = 0,
    SolveResultUnsolvable = 1,
    SolveResultNotFound = 2,
    SolveResultInvalidSrc = 3,
    SolveResultInvalidTgt = 4,
    SolveResultUnknownErr = 5
};

#ifdef __cplusplus
extern "C" {
#endif

CUBE_EXPORT_FORCE inline const char *solve_result_to_string(SolveResult sr) {
    switch(sr) {
        case SolveResultSuccess:    return "Success.";
        case SolveResultUnsolvable: return "The cube configuration is unsolvable.";
        case SolveResultNotFound:   return "No solution found within the step limit.";
        case SolveResultInvalidSrc: return "Invalid source color configuration.";
        case SolveResultInvalidTgt: return "Invalid target color configuration.";
        case SolveResultUnknownErr: return "Unknown error.";
        default:                    return "???";
    }
}

/*! 
 * @brief solve the Rubic's cube
 * @param src       source color configuration, `NULL` means `id`
 * @param tgt       target color configuration, `NULL` means `id`
 * @param solution  the sequence of moves 
 * @param step      the max steps to search (30 is recommended;)
 * @param best      try its best to find the short (but slower) solution
 * @param formated  0 => raw moves (sequennce of char = 1..18 representing move U..B');
 *                  1 => solution is maneuver formatted (sequence of U..B' separated by space)              
 * @return see enum ``SolveResult``, ``solve_result_to_string``.        
 */
CUBE_EXPORT SolveResult solve_ultimate(
    const char *src, const char* tgt, char* solution_buffer, int step, bool best, int formated);

// i.e., solve_ultimate(src,NULL,buf,30,best,1)
CUBE_EXPORT SolveResult solve(
    const char *src, char* solution_buffer, bool best
);

/* check the solvability of color configuration ( 0 - unsolvable; 1 - solvable ) */
CUBE_EXPORT bool solvable(const char* color_cube);

/* transform cube's color configuration by the maneuver */
CUBE_EXPORT void facecube(const char *cube, const char* maneuver, char* cube_buffer);

/* set the (decomposited) permutation of cubies by the maneuver */
CUBE_EXPORT void permutation(const char* maneuver, char* perm_buffer);

#ifdef __cplusplus
} // extern "C"
#endif