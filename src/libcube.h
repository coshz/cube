#pragma once

////! The buffer parameter of function are presumed to be large enough; ////
////! buffer size is at least `LIBCUBE_BS`.                             ////

#ifndef LIBCUBE_BS
#define LIBCUBE_BS 80
#else 
#error "`LIBCUBE_BS` is already defined externally"
#endif

/* conversion from Move to String */
extern const char* Move2Str[18]; 

#ifdef __cplusplus
extern "C" {
#endif

/*! 
 * @brief solve the Rubic's cube
 * @param src, tgt: source-target cube of color configuration, `NULL` => cid
 * @param solution: sequence of moves (range: 1~18)
 * @param step: the max step to search (heuristically, 30 is recommended)
 * @return status_code: 0 - success, 1 - unsolvable, 2 - solution not found, 
 *                     -1 - invalid source, -2 - invalid target.             
 */
int solve(const char *src, const char* tgt, char* solution_buffer, int step);

/* set the color configuration by the maneuver sequence */
void facecube(const char* maneuver, char* cube_buffer);

/* set the permutation of cubies by the maneuver sequence */
void permutation(const char* maneuver, char* perm_buffer);

/* check the solvability of cube represented by color configuration */
bool solvable(const char* cube);

#ifdef __cplusplus
} // extern "C"
#endif