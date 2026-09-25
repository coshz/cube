#pragma once
#include "data.hpp"

#include <string_view>
#include <string>
#include <vector>

namespace cube::internal
{
using namespace cube::data;

/* apply maneuver to cube state */
std::string apply_maneuver(std::string_view s, const std::vector<TurnMove>& ms);

/*!
 * \brief Convert maneuver string to sequence of TurnMove.
 *
 * \remark Examples:
 *      - `"U F2 D F'"` => {Ux1,Fx2,Dx1,Fx3};
 *      - `"(UR){2} F"` => {Ux1,Rx1,Ux1,Rx1,Fx1};
 */
auto parse_maneuver(std::string_view s) -> std::vector<TurnMove>;

inline auto operator""_Tm(const char* ts, size_t n) -> std::vector<TurnMove>
{
    return parse_maneuver(std::string(ts,n));
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
bool is_valid_maneuver(std::string_view);

} // namespace cube::internal

