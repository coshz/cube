#include "cube/cube.h"
#include "cube/cube.hh"

#include <cstring>
#include <string_view>

extern "C" {

const char* solve_result_to_string(int sr) 
{ 
    return solve_result_info(static_cast<SolveResult>(sr));
}
    
SolveResult solve(
    char* buf, const char *src, const char* tgt, int step, bool best)
{
    if(!buf) return SolveResultUnknownErr;
    auto s_src = src == NULL ? cube::cubeId : std::string_view(src);
    auto s_tgt = tgt == NULL ? cube::cubeId : std::string_view(tgt);
    auto res = cube::solve(s_src, s_tgt, step, best);
    if(res.is_success()) {
        const auto& s = res.maneuver;
        std::memcpy(buf,s.c_str(),s.size()+1);
        return SolveResultSuccess;
    }
    return static_cast<SolveResult>(res.status);
}

bool solvable(const char* cube)
{
    return cube::is_solvable(cube ? cube : cube::cubeId);
}

bool facecube(char* buf, const char *maneuver, const char *cube)
{
    if(!buf) return false;
    try {
        auto s = cube::apply_maneuver(maneuver, cube);
        std::memcpy(buf, s.c_str(), s.size()+1);
        return true;
    } catch(...) {
        buf[0] = '\0';
        return false;
    }
}

bool permutation(char* buf, const char* ms_or_cube, int format)
{
    if(!buf) return false;
    try {
        auto s = cube::show_permutation(
            ms_or_cube ? ms_or_cube : cube::cubeId,
            static_cast<cube::PermFormat>(format)
        );
        std::memcpy(buf, s.c_str(), s.size()+1);
        return true;
    } catch(...) {
        buf[0] = '\0';
        return false;
    }
}
} // extern "C"