#include "maneuver.hh"
#include "data.hpp"
#include "rubik.hh"
#include <cassert>
#include <cstring>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cube {

using namespace cube::data;

namespace internal {

std::string apply_maneuver(std::string_view s, const std::vector<TurnMove> &ms)
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

auto parse_maneuver(std::string_view s) -> std::vector<TurnMove>
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

bool is_valid_maneuver(std::string_view s)
{
    static const std::regex pat(
        R"(\s*(([UDLRFB]['23]?|\(([UDLRFB]['23]?\s*)+\)(\{\d+\})?)\s*)*)"
    );
    return std::regex_match(s.begin(), s.end(), pat);
}

} // namespace cube::internal
} // namespace cube