#pragma once 
#include "def.h"

#include <cstdlib>
#include <set>
#include <regex>
#include <string>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <optional>
#include <utility>
#include <type_traits>

#if VERBOSE
    #define VPRINT(...) printf(__VA_ARGS__)
#else
    #define VPRINT(...)
#endif

/* convert a sequence to string */
template<typename VectorLike> 
inline std::string seq2str(const VectorLike &xs,
    std::string pre="", std::string suf="", std::string sep="")
{
    if(xs.size() == 0) return std::string("");
    std::stringstream ss;
    ss << pre << xs[0];
    for(size_t i = 1; i < xs.size(); i++) ss << sep << xs[i];
    ss << suf;
    return ss.str();
}

/* get the system cache directory */
inline auto get_cache_dir() -> std::filesystem::path 
{
#ifdef _WIN32
    const char* localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData) return std::filesystem::path(localAppData);
#elif __APPLE__
    const char* home = std::getenv("HOME");
    if (home) return std::filesystem::path(home) / "Library" / "Caches";
#else 
    const char* xdgCache = std::getenv("XDG_CACHE_HOME");
    if (xdgCache) return std::filesystem::path(xdgCache);
    const char* home = std::getenv("HOME");
    if (home) return std::filesystem::path(home) / ".cache";
#endif
    throw std::runtime_error("Unable to determine cache path");
}

/* measure the execution time, for benchmark */
template<typename F, typename... Args>
inline auto time_execution(F&& f, Args&&... args)
{
    using Rf = std::invoke_result_t<F, Args...>;

    auto start = std::chrono::high_resolution_clock::now();
    auto duration_from_start = [start](){
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    };

    if constexpr (std::is_void_v<Rf>) {
        std::forward<F>(f)(std::forward<Args>(args)...);
        return std::make_pair(
            duration_from_start(), 
            std::nullopt
        );
    } else {
        auto result = std::forward<F>(f)(std::forward<Args>(args)...);
        return std::make_pair(
            duration_from_start(),
            std::make_optional(result)
        );
    }
}

/*!
 * @brief check the validity of color configuration of cube
 * @def A configuration is valid ::= it's legal (solvable) modulo edge flips orcorner twists
 */
template<typename VectorLike>
bool is_valid_config(const VectorLike &cfg)
{
    // check size
    if(cfg.size() != 54) return false;
        
    // check centers
    std::set<char> vs { cfg[CC[0]],cfg[CC[1]],cfg[CC[2]],cfg[CC[3]],cfg[CC[4]],cfg[CC[5]] };
    if(vs.size() != 6) return false;

    // check cubies
    for(size_t i = 0, x = 0; i < 8; i++) {
        for(x = 0; x < 24; x++) {
            if(cfg[CC[CCI[i][0]]] == cfg[CF[x/3][x%3]] 
               && cfg[CC[CCI[i][1]]] == cfg[CF[x/3][(x+1)%3]]
               && cfg[CC[CCI[i][2]]] == cfg[CF[x/3][(x+2)%3]]) break;
        }
        if(x >= 24) return false;
    }
    for(size_t i = 0, y = 0; i < 12; i++) {
        for(y = 0; y < 24; y++) {
            if(cfg[CC[ECI[i][0]]] == cfg[EF[y/2][y%2]]
               && cfg[CC[ECI[i][1]]] == cfg[EF[y/2][(y+1)%2]]) break;
        }
        if(y >= 24) return false;
    }
    return true;
}

/*
  Convert move string to sequence of TurnAxis or TurnMove.
  A valid move string is a sequence of terms seperated by optional spaces, where
    - a term is one of: Y, (Y), (Y){n}; (Y: an atom, n: an integer);
    - an atom is one of: X, X2, X3, X'; (X: one of U,D,L,R,F,B);

  Note: nested brackets are prohibited.
    
  Example: ("=>" means "is interpreted as"):
    - `"U F2 D F'"` => (TurnMove) `{Ux1,Fx2,Dx1,Fx3}`; (TurnAxis) `{U,F,F,D,F}`
    - `"(UR){2} F"` => (TurnMove) `{Ux1,Rx1,Ux1,Rx1,Fx1}`; (TurnAxis) `{U,R,U,R,F}`
 */
template<typename Out, 
    std::enable_if_t<std::is_same_v<Out, TurnMove> || std::is_same_v<Out, TurnAxis>, int> = 0 >
std::vector<Out> string_to_moves(std::string s)
{
    // expand patterns 
    // Y => Y, (Y){n} => Y...Y, (Y) => Y
    auto expand = [] (std::string in) -> std::string {
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
        }
        return res;
    };

    // map to TurnMove
    std::string in = expand(s);
    std::vector<Out> ms;
    ms.reserve(in.size());
    for(int i = 0; i < in.size(); ++i) {
        if constexpr (std::is_same_v<Out,TurnMove>) {
            switch(in[i]) {
            case ' ':   break;
            case '2':   if(!ms.empty()) ms.back() = static_cast<Out>(ms.back() + 1); break;
            case '3':   
            case '\'':  if(!ms.empty()) ms.back() = static_cast<Out>(ms.back() + 2); break;
            case 'U':   ms.push_back(Ux1); break;
            case 'R':   ms.push_back(Rx1); break;
            case 'F':   ms.push_back(Fx1); break;
            case 'D':   ms.push_back(Dx1); break;
            case 'L':   ms.push_back(Lx1); break;
            case 'B':   ms.push_back(Bx1); break;
            default:    throw std::invalid_argument(
                std::string("unsupported character `") + in[i] + "`");
            };
        } else {
            switch(in[i]) {
            case ' ':   break;
            case '2':   if(!ms.empty()) ms.push_back(ms.back()); break;
            case '3':   
            case '\'':  if(!ms.empty()) ms.push_back(ms.back()), ms.push_back(ms.back()); break;
            case 'U':   ms.push_back(U); break;
            case 'R':   ms.push_back(R); break;
            case 'F':   ms.push_back(F); break;
            case 'D':   ms.push_back(D); break;
            case 'L':   ms.push_back(L); break;
            case 'B':   ms.push_back(B); break;
            default:    throw std::invalid_argument(
                std::string("unsupported character `") + in[i] + "`");
            };
        }
    }
    return ms;
}

inline std::vector<TurnMove> operator""_Tm(const char* ts, size_t n)
{
    return string_to_moves<TurnMove>(std::string(ts,n));
}

inline std::vector<TurnAxis> operator""_Ta(const char* ts, size_t n)
{
    return string_to_moves<TurnAxis>(std::string(ts,n));
}