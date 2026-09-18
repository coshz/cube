// SPDX-License-Identifier: MIT
// Copyright (c) 2026 coshz

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

#ifdef __cplusplus
    #define Default(x) = x
#else 
    #define Default(x)
#endif

/*******************************************************************************
 @remark Items
 Items in the cube are defined as follows:

    color       :: { U,R,F,D,L,B } 
    move        :: { U,U2,U',R,R2,R',F,F2,F',D,D2,D',L,L2,L',B,B2,B' } (1~18)
    cubie       :: corner (ufl,ubr,...) OR edge (uf,ub,...)
    cube        :: the sequence of color without seperator, eg:
                   cid = `UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`
    maneuver    :: the sequence of move seperated by spaces, eg:
                   "U' F2 L R"
    permutation :: format=face      "U1U2U3...B9"
                   format=cubie     "ABCDEFGH00000000opqrstuvwxyz000000000000"
                   format=cycle     "(ufl,urf,ubr)(uf,ul,ur)(+u)(−d)"
  
    a color configuration is:
        solvable if: it could be obtained by moves onto the identity cube;
        valid if: it is solvable up to edge flips or corner twists.

 @remark Schema

    1. the facelet schema:
   
                     U1 U2 U3 
                     U4 U5 U6
                     U7 U8 U9 
            L1 L2 L3 F1 F2 F3 R1 R2 R3 B1 B2 B3
            L4 L5 L6 F4 F5 F6 R4 R5 R6 B4 B5 B6
            L7 L8 L9 F7 F8 F9 R7 R8 R9 B7 B8 B9
                     D1 D2 D3
                     D4 D5 D6
                     D7 D8 D9
   
   2. the cubie schema:
   
                  ^ U
                  |                            [Corners (A~H)]  
             C ------- r ------- D                 A: URF    E: DFR
            /|                  /|                 B: UFL    F: DLF 
           q |                 o |                 C: ULB    G: DBL   
          /  y                /  z                 D: UBR    H: DRB      
         B ------- p ------- A   |  ---> R           
         |   |               |   |             [Edges (o~z)]
         |   G . . . v . . . | . H                 o: UR    s: DR    w: FR
         x  .                w  /                  p: UF    t: DF    x: FL
         | u                 | s                   q: UL    u: DL    y: BL
         |.                  |/                    r: UB    v: DB    z: BR
         F ------- t ------- E
        /
       v F
// ---------------------------------------------------------------------------*/

/*!
 * @brief The buffer size minimum
 * @remark 
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
 * @brief solve the Rubik's cube

 * @param buf       the buffer to solution (space-seperated moves)
 * @param src       source color configuration, `NULL` means `id`
 * @param tgt       target color configuration, `NULL` means `id`
 * @param step      the max steps to search (30 is recommended;)
 * @param best      try its best to find the short (but slower) solution             
 * @return 
 *  see enum ``SolveResult``, ``solve_result_to_string``.        
 */
CUBE_EXPORT SolveResult solve(
    char*       buf, 
    const char* src         Default(CUBE_ID),
    const char* tgt         Default(CUBE_ID),
    int         step        Default(30),
    bool        best        Default(true)
);

/* check the solvability of color configuration */
CUBE_EXPORT bool solvable(
    const char* color_cube  Default(CUBE_ID)
);

/**
 * @brief Applies a sequence of moves (maneuver) to a 54-character color cube.
 * 
 * @param buf           output buffer to store the resulting color cube
 * @param maneuver      moves string (e.g., "R U R' U'"), `NULL` means ""
 * @param cube          color cube source, `NULL` means `CUBE_ID`
 *
 * @return             
 *  false if: cube length != 54, cube_buffer is NULL, or maneuver parsing fails;
 *  true otherwise.
 */
CUBE_EXPORT bool facecube(
    char* buf, 
    const char* maneuver    Default(""),
    const char *cube        Default(CUBE_ID)
);

/*!
 * @brief Computes the permutation state resulting from either a maneuver or 
 *        a color cube, and outputs it in the specified format. 
 * 
 * @param buf         output buffer to store the resulting string
 * @param ms_or_cube  maneuver  eg., "R U R' U'"
 *                    OR cube   e.g. "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
 * @param format      output format selector (default: 2):
 *                      0 => 54-face;   eg. "U1U2U3...B9"
 *                      1 => 20-cubie;  eg. "ABCDEFGH00000000opqrstuvwxyz000000000000"
 *                      2 => cycles:    eg. "(ufl,urf,ubr)(uf,ul,ur)(+u)(−d)"
 * @return 
 *  false if: perm_buffer is NULL, or maneuver parsing fails;
 *  true otherwise.
 */
CUBE_EXPORT bool permutation(
    char*       buf, 
    const char* ms_or_cube  Default(CUBE_ID), 
    int         format      Default(2)
);

#ifdef __cplusplus
} // extern "C"
#endif
