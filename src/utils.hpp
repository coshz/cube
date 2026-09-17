#pragma once
#include "def.h"
#include "help.hpp"

#include <cassert>
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
#include <string_view>

namespace cube::utils {

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
 * @def A configuration is valid ::= it's legal (solvable) modulo edge flips or corner twists
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

/*!
 * @brief check the validity of maneuver
 *
 * @remark
 *   The valid maneuver strings are defined as:
 *      <maneuver>  ::= <Term>*
 *      <Term>      ::= <C> | "(" <C> ")" | "(" <C> ")" "{" <Nat> "}"
 *      <C>         ::= (<X><M>)+
 *      <X>         ::= "U" | "D" | "L" | "R" | "F" | "B"
 *      <M>         ::= ϵ | "2" | "'"
 *      <Nat>       ::= [0-9]+
 *      ϵ           ::= ""
 *  (* note: spaces " " are ignored *)
 */
inline bool is_valid_maneuver(std::string_view s)
{
    static const std::regex pat(
        R"(\s*(([UDLRFB]['23]?|\(([UDLRFB]['23]?\s*)+\)(\{\d+\})?)\s*)*)"
    );
    return std::regex_match(s.begin(), s.end(), pat);
}

/*!
 * \brief Convert maneuver string to sequence of TurnMove.
 *
 * \remark Examples:
 *      - `"U F2 D F'"` => {Ux1,Fx2,Dx1,Fx3};
 *      - `"(UR){2} F"` => {Ux1,Rx1,Ux1,Rx1,Fx1};
 */
inline auto parse_manuever(std::string_view s) -> std::vector<TurnMove>
{
    assert(is_valid_maneuver(s) && "invalid maneuver");
   
    // expand: (Y){n} => Y...Y
    auto expand = [](std::string_view in) -> std::string {
        static const std::regex group_re(R"(\(([^)]+)\)(?:\{(\d+)\})?)");
        std::string res;
        auto start = in.cbegin();
        std::match_results<std::string_view::const_iterator> m;

        while (std::regex_search(start, in.cend(), m, group_re)) {
            res.append(start, m[0].first);
            int repeat = m[2].matched ? std::stoi(m[2].str()) : 1;
            for (int i = 0; i < repeat; ++i) res += m[1].str();
            start = m[0].second;
        }
        res.append(start, in.cend());
        return res;
    };

    auto char_to_move = [](char c) -> TurnMove {
        switch(c) {
        case 'U': return Ux1;
        case 'R': return Rx1;
        case 'F': return Fx1;
        case 'D': return Dx1;
        case 'L': return Lx1;
        case 'B': return Bx1;
        default: throw std::invalid_argument("char_to_move: ???");
        }
    };

    std::string in = expand(s);
    std::vector<TurnMove> ms;
    ms.reserve(in.size());

    for (char c : in)
    {
        switch (c) {
        case ' ': break;
        case '2':
            ms.back() = static_cast<TurnMove>(ms.back() + 1);
            break;
        case '\'':
            ms.back() = static_cast<TurnMove>(ms.back() + 2);
            break;
        default:
            ms.push_back(char_to_move(c));
            break;
        }
    }

    return ms;
}

inline std::vector<TurnMove> operator""_Tm(const char* ts, size_t n)
{
    return parse_manuever(std::string(ts,n));
}

} // namespace cube::utils