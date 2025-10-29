#include "maneuver.hh"
#include "cube.hh"
#include "twophase.hh"
#include "libcube.h"

const char* Move2Str[18] = { "U","U2","U'","R","R2","R'","F","F2","F'","D","D2","D'","L","L2","L'","B","B2","B'" };
const std::string cid = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

TwoPhaseSolver TPS;

int solve(const char *src, const char* tgt, char* solution_buffer, int step)
{
    auto s_src = src == NULL ? cid : std::string(src);
    auto s_tgt = tgt == NULL ? cid : std::string(tgt);

    // invalid cube
    if(!isValidConfig(s_src)) return -1;
    if(!isValidConfig(s_tgt)) return -2;

    auto cc_src = (s_src == cid) ? CubieCube::id : CubieCube::fromString(s_src);
    auto cc_tgt = (s_tgt == cid) ? CubieCube::id : CubieCube::fromString(s_tgt);
    
    // trivial cube
    if(cc_src == cc_tgt) { solution_buffer[0] = '\0'; return 0; }

    CubieCube cc = ~cc_tgt*cc_src;

    // unsolvable
    if(!cc.isSolvable()) return 1;
    
    const auto & [found, s1, s2] = TPS.solve(Coord::CubieCube2Coord(cc), step);
    // const auto & [found, s1, s2] = TPS.solve(cc, step);

    // solution is not found since the search depth is too small
    if(!found) return 2;
    
    std::vector<TurnMove> sol = [](const std::vector<TurnMove> &s1, const std::vector<TurnMove> &s2) {
        std::vector<TurnMove> solution;
        size_t n1 = s1.size(), n2 = s2.size();
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

    for(size_t i = 0; i < sol.size(); i++) {
        solution_buffer[i] = static_cast<char>(1 + sol[i]); // offset 1 to avoid terminated 0 
    }
    solution_buffer[sol.size()] = '\0';

    return 0;
}

bool solvable(const char* cube)
{
    return CubieCube::fromString(std::string(cube)).isSolvable();
}

void facecube(const char* maneuver, char* cube_buffer)
{
    auto m = Maneuver(std::string(maneuver));
    auto s = m.fc_color();
    std::copy(s.cbegin(), s.cend(), cube_buffer);
    cube_buffer[s.length()] = '\0';
}

void permutation(const char* maneuver, char* perm_buffer)
{
    auto m = Maneuver(std::string(maneuver));
    auto s = m.cc_permutation();
    std::copy(s.cbegin(), s.cend(), perm_buffer);
    perm_buffer[s.length()] = '\0';
}