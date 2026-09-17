#pragma once
#include "def.h"
#include "data.hpp"
#include "utils.hpp"
#include <string_view>
#include <array>

namespace cube {

using namespace cube::data;
using namespace cube::utils;

typedef int8_t cube_value_t;
typedef Perm<54,    cube_value_t>   FacePerm;
typedef Perm<8,     cube_value_t>   CornerPerm;
typedef Perm<12,    cube_value_t>   EdgePerm;
typedef CArray<3, 8,cube_value_t>   CornerOri;
typedef CArray<2,12,cube_value_t>   EdgeOri;

struct ColorState;
struct FaceCube;
struct CubieCube;

constexpr CornerPerm operator*(const CornerPerm &cp, const CubieCube &cc);
constexpr CornerOri  operator*(const CornerOri  &co, const CubieCube &cc);
constexpr EdgePerm   operator*(const EdgePerm   &ep, const CubieCube &cc);
constexpr EdgeOri    operator*(const EdgeOri    &eo, const CubieCube &cc);

struct ColorState
{
    std::array<Layer,54> s;

    Layer & operator[](size_t i) noexcept           { return s[i]; }
    const Layer operator[](size_t i) const noexcept { return s[i]; }

    // ? Should we make toFaceCube / toCubieCube constexpr

    static ColorState fromString(std::string_view cube);

    FaceCube  toFaceCube() const;
    CubieCube toCubieCube() const;

    static const ColorState id;
};

inline const ColorState ColorState::id = {{U,U,U,U,U,U,U,U,U,R,R,R,R,R,R,R,R,R,F,F,F,F,F,F,F,F,F,D,D,D,D,D,D,D,D,D,L,L,L,L,L,L,L,L,L,B,B,B,B,B,B,B,B,B}};

/*!
 * @brief FaceCube: the Rubik's group based on permutations of 54-facelet
 */
struct FaceCube
{
    FacePerm f;

    CubieCube toCubieCube() const;

    friend constexpr FaceCube operator*(const FaceCube &lhs, const FaceCube &rhs)
    { return { lhs.f * rhs.f }; }

    constexpr FaceCube& operator*=(const FaceCube &rhs)
    { return *this = *this * rhs; }

    constexpr FaceCube operator~() const
    { return { ~f }; }

    friend bool operator==(const FaceCube &lhs, const FaceCube &rhs)
    { return lhs.f == rhs.f; }

    static const FaceCube id;
};

/*!
 * @brief CubieCube: the Rubik's group based on permutations of 20-cubie
 */
struct CubieCube
{
    CornerPerm  cp;
    CornerOri   co;
    EdgePerm    ep;
    EdgeOri     eo;

    FaceCube toFaceCube() const;

    bool isSolvable() const
    {  return cp.parity() == ep.parity() && co.sum() == 0 && eo.sum() == 0; }

    friend constexpr CubieCube operator*(const CubieCube &a, const CubieCube &b)
    { return { a.cp*b, a.co*b, a.ep*b, a.eo*b }; }

    constexpr CubieCube& operator*=(const CubieCube &rhs)
    { return *this = *this * rhs; }

    constexpr CubieCube operator~() const
    {
        auto cp_inv = ~cp;
        auto ep_inv = ~ep;
        return { cp_inv, ~(co*cp_inv), ep_inv, ~(eo*ep_inv) };
    }

    friend bool operator==(const CubieCube &lhs, const CubieCube &rhs)
    { return lhs.cp == rhs.cp && lhs.co == rhs.co && lhs.ep == rhs.ep && lhs.eo == rhs.eo; }
   
    static const CubieCube id;
};

inline constexpr CornerPerm operator*(const CornerPerm &cp, const CubieCube &cc) { return cp * cc.cp;}
inline constexpr CornerOri  operator*(const CornerOri &co,  const CubieCube &cc) { return co * cc.cp + cc.co; }
inline constexpr EdgePerm   operator*(const EdgePerm &ep,   const CubieCube &cc) { return ep * cc.ep; }
inline constexpr EdgeOri    operator*(const EdgeOri &eo,    const CubieCube &cc) { return eo * cc.ep + cc.eo; }

/* identity of CornerPerm, EdgePerm, CornerOri, EdgeOri */

inline constexpr CornerPerm    eCP = {0,1,2,3,4,5,6,7};
inline constexpr EdgePerm      eEP = {0,1,2,3,4,5,6,7,8,9,10,11};
inline constexpr CornerOri     eCO = {0,0,0,0,0,0,0,0};
inline constexpr EdgeOri       eEO = {0,0,0,0,0,0,0,0,0,0,0,0};
inline constexpr FacePerm      eFP = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53};

/* identity of FaceCube, CubieCube */

inline const FaceCube   FaceCube::id = FaceCube{eFP};
inline const CubieCube  CubieCube::id = CubieCube{eCP,eCO,eEP,eEO};

/* CubieCube representations of 18 elementary moves: used for pdb::Coord & pdb::Table */

inline constexpr CubieCube mU = {{3,0,1,2,4,5,6,7},{0,0,0,0,0,0,0,0},{3,0,1,2,4,5,6,7,8,9,10,11},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr CubieCube mR = {{4,1,2,0,7,5,6,3},{2,0,0,1,1,0,0,2},{8,1,2,3,11,5,6,7,4,9,10,0},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr CubieCube mF = {{1,5,2,3,0,4,6,7},{1,2,0,0,2,1,0,0},{0,9,2,3,4,8,6,7,1,5,10,11},{0,1,0,0,0,1,0,0,1,1,0,0}};
inline constexpr CubieCube mD = {{0,1,2,3,5,6,7,4},{0,0,0,0,0,0,0,0},{0,1,2,3,5,6,7,4,8,9,10,11},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr CubieCube mL = {{0,2,6,3,4,1,5,7},{0,1,2,0,0,2,1,0},{0,1,10,3,4,5,9,7,8,2,6,11},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr CubieCube mB = {{0,1,3,7,4,5,2,6},{0,0,1,2,0,0,2,1},{0,1,2,11,4,5,6,10,8,9,3,7},{0,0,0,1,0,0,0,1,0,0,1,1}};

inline constexpr std::array<CubieCube,18>
    ElementaryMove = { mU,mU*mU,mU*mU*mU,mR,mR*mR,mR*mR*mR,mF,mF*mF,mF*mF*mF,mD,mD*mD,mD*mD*mD,mL,mL*mL,mL*mL*mL,mB,mB*mB,mB*mB*mB };

/* FaceCubie representations of 18 elementary moves: used for ColorState */

inline constexpr FaceCube pU = {{6,3,0,7,4,1,8,5,2,45,46,47,12,13,14,15,16,17,9,10,11,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,18,19,20,39,40,41,42,43,44,36,37,38,48,49,50,51,52,53}};
inline constexpr FaceCube pR = {{0,1,20,3,4,23,6,7,26,15,12,9,16,13,10,17,14,11,18,19,29,21,22,32,24,25,35,27,28,51,30,31,48,33,34,45,36,37,38,39,40,41,42,43,44,8,46,47,5,49,50,2,52,53}};
inline constexpr FaceCube pF = {{0,1,2,3,4,5,44,41,38,6,10,11,7,13,14,8,16,17,24,21,18,25,22,19,26,23,20,15,12,9,30,31,32,33,34,35,36,37,27,39,40,28,42,43,29,45,46,47,48,49,50,51,52,53}};
inline constexpr FaceCube pD = {{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,24,25,26,18,19,20,21,22,23,42,43,44,33,30,27,34,31,28,35,32,29,36,37,38,39,40,41,51,52,53,45,46,47,48,49,50,15,16,17}};
inline constexpr FaceCube pL = {{53,1,2,50,4,5,47,7,8,9,10,11,12,13,14,15,16,17,0,19,20,3,22,23,6,25,26,18,28,29,21,31,32,24,34,35,42,39,36,43,40,37,44,41,38,45,46,33,48,49,30,51,52,27}};
inline constexpr FaceCube pB = {{11,14,17,3,4,5,6,7,8,9,10,35,12,13,34,15,16,33,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,36,39,42,2,37,38,1,40,41,0,43,44,51,48,45,52,49,46,53,50,47}};

inline constexpr std::array<FaceCube,18>
    ElementaryPerm = { pU,pU*pU,pU*pU*pU,pR,pR*pR,pR*pR*pR,pF,pF*pF,pF*pF*pF,pD,pD*pD,pD*pD*pD,pL,pL*pL,pL*pL*pL,pB,pB*pB,pB*pB*pB };

inline FaceCube operator*(const FaceCube &c, const std::vector<TurnMove> &ms)
{
    FaceCube fc = c;
    for (auto m : ms) fc = fc * ElementaryPerm[m];
    return fc;
}

inline CubieCube operator*(const CubieCube &c, const std::vector<TurnMove> &ms)
{
    CubieCube cc = c;
    for (auto m : ms) cc = cc * ElementaryMove[m];
    return cc;
}

inline ColorState operator*(const ColorState &c, const std::vector<TurnMove> &ms)
{
    //? Ping-Pong Buffering Optimization
    ColorState cs = c;
    for (auto m : ms) cs = ElementaryPerm[m].f.act_right(cs);
    return cs;
}

} // namespace cube