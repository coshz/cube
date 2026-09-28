#include <cube/cube.h>
#include <stdio.h>
#include <string.h>

int main() 
{
  char buf[CUBE_BS], sol[CUBE_BS];

  // apply maneuver to give cube (here, CUBE_ID)
  facecube(buf, "U F U' L2 R L' D2 B", CUBE_ID);

  // solve from buf to CUBE_ID
  SolveResult sr = solve(sol, buf, CUBE_ID, 30, true);

  if(sr == SolveResultSuccess) {
    puts(sol);
    // verify the solution 
    char solved[CUBE_BS];
    facecube(solved, sol, buf);
    if(strcmp(solved, CUBE_ID) == 0) {
      puts("solution verified");
    } else {
      puts("wrong solution");
    }
  } else {
    fprintf(stderr, "%s\n", solve_result_to_string(sr));
  }
  return 0;
}