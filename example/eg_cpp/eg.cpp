#include <cube/cube.hh>
#include <iostream>

int main() 
{
  auto cube = cube::apply_maneuver("U F U' L2 R L' D2 B");
  auto solution = cube::solve(cube);
  if(solution.is_success()) {
    std::cout << "Solution: " << solution.maneuver << std::endl;
    if(cube::cubeId == cube::apply_maneuver(solution.maneuver,cube)) {
      std::cout << "solution verified" << std::endl;
    } else {
      std::cout << "wrong solution" << std::endl;
    }
  } else {
    std::cerr << cube::to_string(solution.status) << std::endl;
  }
}