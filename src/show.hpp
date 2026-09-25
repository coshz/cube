#include "rubik.hh"
#include "common.hpp"
#include "data.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
#include <array>
#include <type_traits>
#include <vector>

namespace cube::show {
    
using std::size_t;
using namespace cube::data;

template<size_t N>
using StringArr = std::array<std::string_view,N>;

inline constexpr StringArr< 8> CornerToString       = { "urf", "ufl", "ulb", "ubr", "dfr", "dlf", "dbl", "drb" };
inline constexpr StringArr<12> EdgeToString         = { "ur","uf","ul","ub","dr","df","dl","db","fr","fl","bl","br" };
inline constexpr StringArr< 6> CenterToString       = { "u", "r", "f", "d", "l", "b" };
inline constexpr StringArr< 3> OrientationToString  = { "", "+", "-" };
inline constexpr StringArr<18> Move2Str             = { "U","U2","U'","R","R2","R'","F","F2","F'","D","D2","D'","L","L2","L'","B","B2","B'" };
inline constexpr StringArr<54> Face2Str             = { "U1","U2","U3","U4","U5","U6","U7","U8","U9","R1","R2","R3","R4","R5","R6","R7","R8","R9","F1","F2","F3","F4","F5","F6","F7","F8","F9","D1","D2","D3","D4","D5","D6","D7","D8","D9","L1","L2","L3","L4","L5","L6","L7","L8","L9","B1","B2","B3","B4","B5","B6","B7","B8","B9" };
inline constexpr std::string_view ColorSet          = "URFDLB";
inline constexpr std::string_view CornerSet         = "ABCDEFGH";
inline constexpr std::string_view EdgeSet           = "opqrstuvwxyz";

enum class CubeFormat {
    Face,
    Cubie,
    Cycle
};

/*!
 * \brief convert a sequence to string
 *
 * \tparam Container the type of sequence
 * \tparam Formatter the function to format each element
 *
 * \return the formatted string
 */
template<typename Array, typename Formatter>
inline std::string seq2str_fmt(const Array &xs,
                           Formatter && fmt,
                           std::string_view sep="",
                           std::string_view pre="",
                           std::string_view suf="")
{
    const size_t len = xs.size();
    if(len == 0) { return std::string(pre) + std::string(suf); }

    std::stringstream ss;
    ss << pre << fmt(xs[0]);
    for(size_t i = 1; i < len; ++i) ss << sep << fmt(xs[i]);
    ss << suf;
    return ss.str();
}

/*!
 * \brief seq2str with default formatter that promotes int8_t / uint8_t to int
 */
template<typename Container>
inline std::string seq2str(const Container &xs,
                           std::string_view sep="",
                           std::string_view pre="",
                           std::string_view suf="")
{
    auto default_formatter = [](const auto& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T,uint8_t> || std::is_same_v<T,int8_t>) {
            return static_cast<int>(val);
        } else {
            return val;
        }
    };

    return seq2str_fmt(xs, default_formatter, sep, pre, suf);
}

inline std::string to_string(const ColorState& cs)
{
    return seq2str_fmt(cs.s, [](auto v) {
        switch(v){
        case U: return 'U';
        case R: return 'R';
        case F: return 'F';
        case D: return 'D';
        case L: return 'L';
        case B: return 'B';
        default: throw std::invalid_argument("to_string(ColorState): ???");        
        } 
    });
}

inline std::string to_string(const FaceCube& fc, bool use_digit=true)
{
    if(use_digit){
        return seq2str(fc.f);
    } else {
        return seq2str_fmt(fc.f, [](auto v) { return Face2Str[v]; });
    }
}

inline std::string to_string(const CubieCube& cc, CubeFormat fmt)
{
    switch(fmt){
    case CubeFormat::Face:
        return to_string(cc.toFaceCube(), false);
    case CubeFormat::Cubie:
        return seq2str_fmt(cc.cp, [](auto v) { return CornerSet[v]; }) +
               seq2str_fmt(cc.co, [](auto v) { return static_cast<char>('0'+v); }) +
               seq2str_fmt(cc.ep, [](auto v) { return EdgeSet[v]; }) +
               seq2str_fmt(cc.eo, [](auto v) { return static_cast<char>('0'+v);});
    default: // Cycle
        const auto [fixed_corner,cycles_corner] = decomposite(cc.cp);
        const auto [fixed_edge, cycles_edge ]   = decomposite(cc.ep);
       
        std::string s1, s2;
        s1.reserve(64);
        s2.reserve(64);

        auto append_fixed = [&](std::string& out, const auto& fixed, const auto& ori_arr, auto name_table) {
            for(auto idx: fixed) {
                if(ori_arr[idx] == 0) continue;
                out += '(';
                out += OrientationToString[ori_arr[idx]];
                out += name_table[idx];
                out += ')';
            }
        };

        auto append_cycles = [&](std::string& out, const auto& cycles, const auto& ori_arr, auto name_table) {
            for(const auto& cycle: cycles) {
                const size_t N = cycle.size();
                if(N == 0) continue;
                out += '(';
                for(int i = 0; i < N; i++) {
                    if(i > 0) out += ',';
                    size_t prev_idx = cycle[(N-1+i)%N];
                    out += OrientationToString[ori_arr[prev_idx]];
                    out += name_table[cycle[i]]; 
                }
                out += ')';
            }
        };

        append_fixed(s1, fixed_corner, cc.co, CornerToString);
        append_cycles(s1, cycles_corner, cc.co, CornerToString);
        append_fixed(s2, fixed_edge, cc.eo, EdgeToString);
        append_cycles(s2, cycles_edge, cc.eo, EdgeToString);

        return s1.empty() && s2.empty() ? "id" : s1 + s2;
    }
}

inline std::string to_string(const std::vector<TurnMove>& ms)
{
    return seq2str_fmt(ms, [](auto v){
        return Move2Str[v];
    }, " ");
}

} // namespace cube::show