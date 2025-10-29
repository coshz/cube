#include "cube/libcube.h"
#include "help.hpp"
#include "utils.hpp"
#include "twophase.hh"

const char* CornerToString[8]       = { "urf", "ufl", "ulb", "ubr", "dfr", "dlf", "dbl", "drb" };
const char* EdgeToString[12]        = { "ur","uf","ul","ub","dr","df","dl","db","fr","fl","bl","br" };
const char* CenterToStringp[6]      = { "u", "r", "f", "d", "l", "b" };
const char* OrientationToString[]   = { "", "+", "-" };
const char* Move2Str[18]            = { "U","U2","U'","R","R2","R'","F","F2","F'","D","D2","D'","L","L2","L'","B","B2","B'" };
const char* ColorSet                = "URFDLB";

const std::string cid = CUBE_ID;

TwoPhaseSolver TPS;

std::string moves_to_string(const std::vector<TurnMove> &ms, std::string sep = " ") 
{
    std::string r;
    int N = ms.size();
    if(N > 0) r = r + Move2Str[ms[0]];
    for(int i = 1; i < N; i++) r += sep + Move2Str[ms[i]];
    return r;
}

int solve_ultimate(const char *src, const char* tgt, char* solution_buffer, int step, int best, int formated)
{
    auto s_src = src == NULL ? cid : std::string(src);
    auto s_tgt = tgt == NULL ? cid : std::string(tgt);

    // invalid cube
    if(s_src != cid && !is_valid_config(s_src)) return CODE_INVALID_SRC;
    if(s_tgt != cid && !is_valid_config(s_tgt)) return CODE_INVALID_TGT;

    auto cc_src = (s_src == cid) ? CubieCube::id : CubieCube::fromString(s_src);
    auto cc_tgt = (s_tgt == cid) ? CubieCube::id : CubieCube::fromString(s_tgt);
    
    // trivial cube
    if(cc_src == cc_tgt) { solution_buffer[0] = '\0'; return CODE_OK; }

    CubieCube cc = ~cc_tgt*cc_src;

    // unsolvable
    if(!cc.isSolvable()) return CODE_UNSOLVABLE;
    
    const auto & [found, s1, s2] = TPS.solve(Coord::CubieCube2Coord(cc), step, best);

    // solution is not found since the search depth is too small
    if(!found) return CODE_NOT_FOUND;
    
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

    if(formated == 0) {
        for(size_t i = 0; i < sol.size(); i++) {
            solution_buffer[i] = static_cast<char>(1 + sol[i]); // shift by 1 since 0 terminates the c-string
        }
        solution_buffer[sol.size()] = '\0';
    } else {
        auto s = moves_to_string(sol," ");
        std::copy(s.cbegin(), s.cend(), solution_buffer);
        solution_buffer[s.length()] = '\0';
    }
    return CODE_OK;
}

int solve(const char *src, char* sol_buffer, int best)
{
    return solve_ultimate(src,NULL,sol_buffer,30,best,1);
}

int solvable(const char* cube)
{
    return CubieCube::fromString(std::string(cube)).isSolvable() ? 1 : 0;
}

void facecube(const char *cube, const char *maneuver, char* cube_buffer) 
{
    auto cc = (cube ? CubieCube::fromString(cube) : CubieCube::id) * string_to_moves<TurnMove>(maneuver);
    auto color = cc.color();
    std::copy(color.cbegin(), color.cend(), cube_buffer);
    cube_buffer[color.length()] = '\0';
}

void permutation(const char* maneuver, char* perm_buffer)
{
    auto cc = CubieCube::id * string_to_moves<TurnMove>(maneuver);
    const auto [fixed_corner,cycles_corner] = decomposite(cc.cp);
    const auto [fixed_edge, cycles_edge ] = decomposite(cc.ep);
    
    auto fixed2str = [&cc](const auto &fixed, int ce) -> std::vector<std::string> {
        int N = fixed.size();
        std::vector<std::string> rs(N);
        for(int i = 0; i < N; i++) {
            if(ce == 0 && cc.co[fixed[i]] == 0) continue;
            if(ce == 1 && cc.eo[fixed[i]] == 0) continue;
            std::string r = (ce==0) 
                ? std::string(OrientationToString[cc.co[fixed[i]]]) 
                    + CornerToString[fixed[i]]
                : std::string(OrientationToString[cc.eo[fixed[i]]]) 
                    + EdgeToString[fixed[i]];
            rs.push_back(r);
        }
        return rs;
    };
    auto cycle2str = [&cc](const auto &cycle, int ce) -> std::vector<std::string> {
        int N = cycle.size();
        std::vector<std::string> rs(N);
        for(int i = 0; i < N; i++) {
            rs[i] = (ce == 0)
                ? std::string(OrientationToString[cc.co[cycle[(N-1+i)%N]]]) 
                    + CornerToString[cycle[i]]
                : std::string(OrientationToString[cc.eo[cycle[(N-1+i)%N]]]) 
                    + EdgeToString[cycle[i]];
        }
        return rs;
    };

    std::string s1 = "", s2 = "";
    for(auto &it: fixed2str(fixed_corner, 0)) {
        s1 += seq2str(it, "(",")","");
    }
    for(auto i = 0; i < cycles_corner.size(); i++) {
        s1 += seq2str(cycle2str(cycles_corner[i], 0), "(", ")", ",");
    }
    for(auto &it: fixed2str(fixed_edge, 1)) {
        s1 += seq2str(it, "(",")","");
    }
    for(auto i = 0; i < cycles_edge.size(); i++) {
        s2 += seq2str(cycle2str(cycles_edge[i], 1), "(", ")", ",");
    }
    std::string perm_str = (s1 == "" && s2 == "") ? "id" : s1 + s2;
    std::copy(perm_str.cbegin(), perm_str.cend(), perm_buffer);
    perm_buffer[perm_str.length()] = '\0';
}