#include "cube/cube.hh"
#include "cube/advanced.hh"
#include "maneuver.hh"
#include "data.hpp"
#include "twophase.hh"
#include "rubik.hh"
#include "coord.hh"
#include "config.hh"
#include "table.hh"
#include "show.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <cassert>

namespace cube {
using namespace cube::data;
namespace fs = std::filesystem;

void set_table_dir(fs::path dir) { config::set_table_dir(dir); }
auto get_table_dir() -> fs::path { return config::get_table_dir(); }

bool tables_ready() { return pdb::tables_ready(); }
void preload_tables() { pdb::preload_tables(); }

bool is_valid_config(std::string_view cube) { return ColorState::is_valid_config(cube); }
bool is_valid_maneuver(std::string_view maneuver) { return internal::is_valid_maneuver(maneuver); }

cube::solver::TwoPhaseSolver TPS;

Solution solve(std::string_view src, std::string_view tgt, int step, bool best) {
    if (src != cubeId && !is_valid_config(src)) return {SolveResult::InvalidSrc, ""};
    if (tgt != cubeId && !is_valid_config(tgt)) return {SolveResult::InvalidTgt, ""};

    // trivial cube
    if (src == tgt) return {SolveResult::Success, ""};

    auto cc_src = ColorState::fromString(src).toCubieCube();
    auto cc_tgt = ColorState::fromString(tgt).toCubieCube();
    CubieCube cc = ~cc_tgt * cc_src;

    // unsolvable cube
    if (!cc.isSolvable()) return {SolveResult::Unsolvable, ""};

    const auto& [found, s1, s2] = TPS.solve(pdb::Coord::CubieCube2Coord(cc), step, best);

    // solution is not found since the search depth is too small
    if (!found) return {SolveResult::NotFound, ""};

    std::vector<TurnMove> sol;
    size_t n1 = s1.size(), n2 = s2.size();
    
    // if the transition moves of ph1-ph2 are homogeneous, combine them
    if (!s1.empty() && !s2.empty() && s1[n1 - 1] / 3 == s2[0] / 3) {
        sol.insert(sol.end(), s1.begin(), s1.end() - 1);
        int m = (s1[n1 - 1] + s2[0] - s2[0] / 3 * 6 + 2) % 4;
        if (m != 0) sol.push_back(static_cast<TurnMove>(s2[0] / 3 * 3 + m - 1));
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

std::string apply_maneuver(std::string_view maneuver, std::string_view cube_state) {
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
        throw std::invalid_argument("Input is neither a valid configuration nor a valid maneuver.");
    }

    auto cc = ColorState::fromString(cube_str).toCubieCube();
    return show::to_string(cc, show::CubeFormat(static_cast<int>(format)));
}

} // namespace cube