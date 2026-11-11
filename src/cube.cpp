#include "cube/cube.hh"
#include "config.hh"
#include "cube/advanced.hh"
#include "data.hpp"
#include "kociemba.hh"
#include "maneuver.hh"
#include "rubik.hh"
#include "show.hpp"
#include "table_provider.hh"
#include <cassert>
#include <filesystem>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cube {
using namespace cube::data;
using namespace cube::pdb;
using namespace cube::solver;

namespace fs = std::filesystem;

void set_table_dir(fs::path dir) { config::set_table_dir(dir); }
auto get_table_dir() -> fs::path { return config::get_table_dir(); }

bool is_valid_config(std::string_view cube) {
  return ColorState::is_valid_config(cube);
}
bool is_valid_maneuver(std::string_view maneuver) {
  return internal::is_valid_maneuver(maneuver);
}

void cube_init(bool use_symmetry, std::string_view dir) {
  static std::once_flag once;
  std::call_once(once, [=]() {
    config::use_symmetry(use_symmetry);
    if (!dir.empty())
      config::set_table_dir(dir);
    if (use_symmetry)
      table::TableProvider::make<true>();
    else
      table::TableProvider::make<false>();
  });
}

Solution solve(std::string_view src, std::string_view tgt, int good) {
  if (src != cubeId && !is_valid_config(src))
    return {SolveResult::InvalidSrc, ""};
  if (tgt != cubeId && !is_valid_config(tgt))
    return {SolveResult::InvalidTgt, ""};

  // trivial cube
  if (src == tgt)
    return {SolveResult::Success, ""};

  auto cc_src = ColorState::fromString(src).toCubieCube();
  auto cc_tgt = ColorState::fromString(tgt).toCubieCube();
  CubieCube cc = ~cc_tgt * cc_src;

  // unsolvable cube
  if (!cc.isSolvable())
    return {SolveResult::Unsolvable, ""};

  cube_init(CUBE_USE_SYMMETRY);

  const auto &[found, s1, s2] = KociembaSolver::solve(cc, good < 0 ? 0 : good);

  if (!found)
    return {SolveResult::NotFound, ""};

  // concat Ph1 & Ph2 solutions
  std::vector<TurnMove> sol;
  const size_t n1 = s1.size(), n2 = s2.size();

  // if the transition moves of ph1-ph2 are homogeneous, combine them
  if (!s1.empty() && !s2.empty() && s1[n1 - 1] / 3 == s2[0] / 3) {
    sol.insert(sol.end(), s1.begin(), s1.end() - 1);
    int m = (s1[n1 - 1] + s2[0] - s2[0] / 3 * 6 + 2) % 4;
    if (m != 0)
      sol.push_back(static_cast<TurnMove>(s2[0] / 3 * 3 + m - 1));
    sol.insert(sol.end(), s2.begin() + 1, s2.end());
  } else {
    sol.insert(sol.end(), s1.begin(), s1.end());
    sol.insert(sol.end(), s2.begin(), s2.end());
  }

  return {SolveResult::Success, show::to_string(sol)};
}

bool is_solvable(std::string_view color_cube) {
  return is_valid_config(color_cube) &&
         ColorState::fromString(color_cube).toCubieCube().isSolvable();
}

std::string apply_maneuver(std::string_view maneuver,
                           std::string_view cube_state) {
  if (cube_state.size() != 54 || !is_valid_maneuver(maneuver)) {
    throw std::invalid_argument("Invalid cube format or maneuver string.");
  }
  const auto ms = internal::parse_maneuver(maneuver);
  return internal::apply_maneuver(cube_state, ms);
}

std::string show_permutation(std::string_view ms_or_cube, PermFormat format) {
  std::string cube_str;

  if (is_valid_config(ms_or_cube)) {
    cube_str = ms_or_cube;
  } else if (is_valid_maneuver(ms_or_cube)) {
    auto ms = internal::parse_maneuver(ms_or_cube);
    cube_str = internal::apply_maneuver(cubeId, ms);
  } else {
    throw std::invalid_argument(
        "Input is neither a valid configuration nor a valid maneuver.");
  }

  auto cc = ColorState::fromString(cube_str).toCubieCube();
  return show::to_string(cc, show::CubeFormat(int(format)));
}

} // namespace cube
