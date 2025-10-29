#include <iostream>
#include <regex>
#include <algorithm>
#include <functional>
#include "maneuver.hh"
#include "help.hpp"

const char* CornerToString[8]       = { "urf", "ufl", "ulb", "ubr", "dfr", "dlf", "dbl", "drb" };
const char* EdgeToString[12]        = { "ur","uf","ul","ub","dr","df","dl","db","fr","fl","bl","br" };
const char* CenterToStringp[6]      = { "u", "r", "f", "d", "l", "b" };
const char* OrientationToString[]   = { "", "+", "-" };
const char* ColorSet                = "URFDLB";


std::vector<TurnAxis> operator""_T(const char* ts, size_t n)
{
    // 1. expand patterns 
    // Y => Y, (Y){n} => Y...Y, (Y) => Y
    auto expand_1 = [] (std::string in, std::string sep="") -> std::string {
        auto re = std::regex(
            "((?:[UDLRFB]['23]?)+)|"                        // Y: group 1
            "(\\(((?:[UDLRFB]['23]?)+)\\)\\{(\\d+)\\})|"    // (Y){n}: group 2,3,4
            "(\\(((?:[UDLRFB]['23]?)+)\\))"                 // (Y): group 5,6
        );

        std::string res = "";
        std::smatch m;
        auto start = in.cbegin();
        auto end = in.cend();
        
        while(true) {
            std::regex_search(start, end, m, re);
            if(m.empty()) break;
            if(m[1].length() != 0) {
                res += m[1].str();
            }
            else if(m[2].length() != 0) {
                for(auto i = 0; i < std::stoi(m[4]); i++) res += m[3].str();
            }
            else if(m[5].length() != 0) {
                res += m[6].str();
            }
            start = m.suffix().first;
            res += sep;
        }
        if(!res.empty()) res = std::string(res.begin(), res.end()-sep.size()); 
        return res;
    };

    // 2. replace Y 
    // X' => XXX, X2 => XX, X3 => XXX
    auto expand_2 = [](std::string in) -> std::string {
        std::string res{};
        res.reserve(in.size());
        for(auto it = in.begin(); it != in.end(); it++) {
            switch(*it) {
            case '2':   res = res + *(it-1); break;
            case '3':
            case '\'':  res = res + *(it-1) + *(it-1); break;
            default:    res = res + *it;
            }
        }
        return res;
    };  

    // 3. character-wise map
    std::string s = expand_2(expand_1(std::string(ts,n)));
    std::vector<TurnAxis> ms(s.length());
    for(auto i = 0; i < ms.size(); i++) 
        for(auto j = 0; j < 6; j++) 
        if(ColorSet[j] == s[i]) { ms[i] = (TurnAxis) j; break; }
    return ms;
}

auto Maneuver::get_facecube() const -> facecube_t&
{
    if(!fc_cached_) {
        for(size_t i = 0; i < 54; i++) {
            size_t j = i;
            for(auto &m: ms) { j = FaceletMove[m][j]; }
            fc[i] = (Face) j;
        }
        fc_cached_ = true;
    }
    return fc;
}

auto Maneuver::get_cubiecube() const -> cubiecube_t&
{
    if(!cc_cached_) {
        for(size_t i = 0; i < 8; i++) {
            size_t j = i, o = 0;
            for(auto rit = ms.rbegin(); rit != ms.rend(); ++rit) {
                o = (o + CornerCubieMove[*rit][j].o) % 3, 
                j = CornerCubieMove[*rit][j].c;
            }
            std::get<0>(cc)[i] = (Corner)j;
            std::get<1>(cc)[i] = o;
        }

        for(size_t i = 0; i < 12; i++){
            size_t j = i, o = 0;
            for(auto rit = ms.rbegin(); rit != ms.rend(); ++rit) {
                o = (o + EdgeCubieMove[*rit][j].o) % 2,
                j = EdgeCubieMove[*rit][j].e;
            }
            std::get<2>(cc)[i] = (Edge)j;
            std::get<3>(cc)[i] = o;
        }
        cc_cached_ = true;
    }
    return cc;
}

auto Maneuver::fc_color() -> std::string
{
    auto fc = get_facecube();
    std::string str_cube(54,' ');
    for(size_t i = 0; i < 54; i++){ str_cube[fc[i]] = ColorSet[i/9]; }
    return str_cube;
}

auto Maneuver::cc_permutation() -> std::string
{
    auto cc = get_cubiecube();
    auto cycles_corner = decomposite(std::get<0>(cc));
    auto cycles_edge = decomposite(std::get<2>(cc));

    // TODO
    // conversion from corner cyles and edge cycles to string have similar logic;
    // however, I haven't conceived a good solution to combine them...

    auto cycle2str = [&cc](const auto &cycle, int ce) -> std::vector<std::string> {
        std::vector<std::string> rs(cycle.size());
        for(size_t i = 0; i < rs.size(); i++) {
            rs[i] = (ce == 0) 
                ? std::string(OrientationToString[std::get<1>(cc)[cycle[i]]]) 
                    + CornerToString[cycle[i]]
                : std::string(OrientationToString[std::get<3>(cc)[cycle[i]]]) 
                    + EdgeToString[cycle[i]];
        }
        return rs;
    };

    std::string s1 = "", s2 = "";

    for(auto i = 1; i < cycles_corner.size(); i++) {
        s1 += seq2str(cycle2str(cycles_corner[i], 0), "(", ")", ",");
    }
    for(auto i = 0; i < cycles_corner[0].size(); i++) {
        if(std::get<1>(cc)[cycles_corner[0][i]] != 0) 
        s1 += seq2str(cycle2str(std::vector<Corner> {cycles_corner[0][i]}, 0), "(", ")", ",");
    }

    for(auto i = 1; i < cycles_edge.size(); i++) {
        s2 += seq2str(cycle2str(cycles_edge[i], 1), "(", ")", ",");
    }
    for(auto i = 0; i < cycles_edge[0].size(); i++) {
        if(std::get<3>(cc)[cycles_edge[0][i]] != 0) 
        s2 += seq2str(cycle2str(std::vector<Edge> {cycles_edge[0][i]}, 1), "(", ")", ",");
    }

    if(s1 == "" && s2 == "") return std::string("I");
    else return s1 + s2;
}