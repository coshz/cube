#pragma once

////! The buffer parameter of function are presumed to be large enough; ////
////! buffer size is at least `CUBE_BS`.                                ////

#ifndef CUBE_BS
    /* CUBE_BS is the default size of buffer to store: 
        - color cube    (* len=54 *);
        - solution      (* len<=30 OR len<=90 formated *);
        - permutation   (* len < 4 * L"(+urf,+ulb)" + 6 * L"(+ur,+ul)" + 6 * L"(++u)" = 4*11+6*9+6*5 = 128 *)
    */
    #define CUBE_BS 128
#else 
    #error "`CUBE_BS` is already defined externally"
#endif

/* the identity of color configuration */
#define CUBE_ID "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"

/*! 
    @brief Terms
    color       :: { U,R,F,D,L,B } 
    move        :: { U,U2,U',R,R2,R',F,F2,F',D,D2,D',L,L2,L',B,B2,B' } (range 1~18)
    cubie       :: corner (ufl,ubr,...) OR edge (uf,ub,...) OR center (u,r,f,d,l,b)
    color_cube  :: the color configuration of cube, eg: cid = `UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`
    maneuver    :: the move sequence, eg: `(DR'F2L){7} BD2`
    permutation :: the cycle-decomposited permutation of cubies, eg: `(ufl,urf,ubr)(uf,ul,ur)(+u)(−d)`
    @remark
*/

enum status_code {
    CODE_OK = 0,
    CODE_UNSOLVABLE = 1,
    CODE_NOT_FOUND = 2,
    CODE_INVALID_SRC = 3,
    CODE_INVALID_TGT = 4,
    CODE_UNKNOWN_ERROR = 5
};

#ifdef __cplusplus
extern "C" {
#endif

/*! 
 * @brief solve the Rubic's cube
 * @param src       source color configuration, `NULL` means `id`
 * @param tgt       target color configuration, `NULL` means `id`
 * @param solution  the sequence of moves 
 * @param step      the max steps to search (30 is recommended;)
 * @param best      try its best to find the short (but slower) solution
 * @param formated  1 => solution is maneuver formatted (sequence of U..B' separated by space); 
 *                  0 => raw moves (sequennce of char = 1..18 representing move U..B')
 * @return status_code: see enum `status_code`.        
 */
int solve_ultimate(const char *src, const char* tgt, char* solution_buffer, int step, int best, int formated);

// solve_ultimate(src,NULL,buf,30,best,1)
int solve(const char *src, char* solution_buffer, int best);

/* check the solvability of color configuration ( 0 - unsolvable; 1 - solvable ) */
int solvable(const char* color_cube);

/* transform cube's color configuration by the maneuver */
void facecube(const char *cube, const char* maneuver, char* cube_buffer);

/* set the (decomposited) permutation of cubies by the maneuver */
void permutation(const char* maneuver, char* perm_buffer);

#ifdef __cplusplus
} // extern "C"
#endif