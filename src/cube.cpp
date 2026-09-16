#include "cube/cube.h"
#include "help.hpp"
#include "show.hpp"
#include "twophase.hh"
#include "utils.hpp"

using namespace cube;

const std::string_view cid = CUBE_ID;

cube::solver::TwoPhaseSolver TPS;

std::string apply_moves(std::string_view s, const std::vector<TurnMove> &ms)
{
    assert(s.size() == 54 && "invalid cube length");

    char buf[2][54];
    std::memcpy(buf[0], s.data(), 54);
    size_t curr = 0;
    for(const auto &m : ms)
    {
        const auto& p   = ElementaryPerm[m].f;
        const char* src = buf[curr];
        char*       dst = buf[1-curr];

        for(int i = 0; i < 54; ++i) dst[i] = src[p[i]];
        curr = 1-curr;
    }
    return std::string(buf[curr],54);
}

SolveResult solve(
    char* buf, const char *src, const char* tgt, int step, bool best)
{
    auto s_src = src == NULL ? cid : std::string_view(src);
    auto s_tgt = tgt == NULL ? cid : std::string_view(tgt);

    // invalid cube
    if(s_src != cid && !cube::utils::is_valid_config(s_src)) return SolveResultInvalidSrc;
    if(s_tgt != cid && !cube::utils::is_valid_config(s_tgt)) return SolveResultInvalidTgt;

    // trivial cube
    if(s_src == s_tgt) { buf[0] = '\0'; return SolveResultSuccess; }

    auto cc_src = ColorState::fromString(s_src).toCubieCube();
    auto cc_tgt = ColorState::fromString(s_tgt).toCubieCube();
    CubieCube cc = ~cc_tgt*cc_src;

    // unsolvable cube
    if(!cc.isSolvable()) return SolveResultUnsolvable;
   
    const auto & [found, s1, s2] = TPS.solve(cube::pdb::Coord::CubieCube2Coord(cc), step, best);

    // solution is not found since the search depth is too small
    if(!found) return SolveResultNotFound;
   
    std::vector<TurnMove> sol = [](const auto &s1, const auto &s2) {
        std::vector<TurnMove> solution;
        size_t n1 = s1.size(), n2 = s2.size();
        // if the transition moves of ph1-ph2 are homogeneous, combine them
        if(!s1.empty() && !s2.empty() && s1[n1-1]/3 == s2[0]/3) {
            std::copy(s1.begin(), s1.end()-1, std::back_inserter(solution));
            int m = (s1[n1-1] + s2[0]- s2[0]/3 *6 +2) %4;
            if(m!=0) solution.push_back(static_cast<TurnMove>(s2[0]/3*3+m-1));
            std::copy(s2.begin()+1, s2.end(), std::back_inserter(solution));
        } else {
            std::copy(s1.begin(), s1.end(), std::back_inserter(solution));
            std::copy(s2.begin(), s2.end(), std::back_inserter(solution));
        }
        return solution;
    }(s1,s2);

    auto s = show::to_string(sol);
    std::copy(s.cbegin(), s.cend(), buf);
    buf[s.length()] = '\0';
    return SolveResultSuccess;
}

bool solvable(const char* cube)
{
    return cube::utils::is_valid_config<std::string_view>(cube)
           && ColorState::fromString(cube).toCubieCube().isSolvable();
}

bool facecube(char* buf, const char *maneuver, const char *cube)
{
    if(!buf) return false;

    std::string cube_str = cube ? std::string(cube) : std::string(CUBE_ID);
    std::string maneuver_str = maneuver ? std::string(maneuver) : std::string("");

    if(cube_str.size()!= 54 || !cube::utils::is_valid_maneuver(maneuver_str)) {
        buf[0] = '\0';
        return false;
    }

    const auto ms = cube::utils::parse_manuever(maneuver_str);
    const auto color = apply_moves(cube_str, ms);

    std::copy(color.cbegin(), color.cend(), buf);
    buf[color.length()] = '\0';
    return true;
}

bool permutation(char* buf, const char* ms_or_cube, int format)
{
    if(!buf) return false;

    std::string cube_str{};
    if(ms_or_cube) {
        if(cube::utils::is_valid_config<std::string_view>(ms_or_cube)) {
            cube_str = std::string(ms_or_cube);
        } else if(cube::utils::is_valid_maneuver(ms_or_cube)) {
            auto ms = cube::utils::parse_manuever(ms_or_cube);
            cube_str = apply_moves(cid,ms);
        } else {
            buf[0] = '\0';
            return false;
        }
    } else {
        cube_str = cid;
    }

    std::string perm_str{};
    auto cc = ColorState::fromString(cube_str).toCubieCube();
    perm_str = show::to_string(cc, show::CubeFormat(format));

    std::copy(perm_str.cbegin(), perm_str.cend(), buf);
    buf[perm_str.length()] = '\0';
    return true;
}