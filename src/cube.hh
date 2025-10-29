#pragma once
#include "def.h"
#include "help.hpp"

typedef int8_t cube_value_t;
typedef Perm<54,cube_value_t>       FacePerm;
typedef Perm<8,cube_value_t>        CornerPerm;
typedef Perm<12,cube_value_t>       EdgePerm;
typedef CArray<3,8,cube_value_t>    CornerOri;
typedef CArray<2,12,cube_value_t>   EdgeOri;

struct FaceCube;
struct CubieCube;

struct FaceCube
{
    constexpr FaceCube(const FacePerm &f_):f{f_}{}
    FaceCube(const CubieCube &cc);
    FaceCube()=default;

    /* string import / export */
    static FaceCube fromString(const std::string &);

    std::string color(std::string cset="URFDLB") const;

    static const FaceCube id;
    FacePerm f;
};

inline bool operator==(const FaceCube &lhs, const FaceCube &rhs)
{
    return lhs.f == rhs.f;
}

struct CubieCube
{
    constexpr CubieCube(const CornerPerm &cp_, const CornerOri &co_, 
              const EdgePerm &ep_, const EdgeOri &eo_)
              :cp{cp_},co{co_},ep{ep_},eo{eo_}{}
    CubieCube(const FaceCube& fc);
    CubieCube()=default;
    
    static CubieCube fromString(const std::string &s) { return CubieCube(FaceCube::fromString(s)); }

    std::string color(std::string cset="URFDLB") const { return FaceCube(*this).color(cset); }

    inline bool isSolvable() const { 
        return cp.parity() == ep.parity() && co.sum() == 0 && eo.sum() == 0;
    }
    
    static const CubieCube id;

    CornerPerm  cp;
    CornerOri   co;
    EdgePerm    ep;
    EdgeOri     eo;
};

inline constexpr CubieCube operator*(const CubieCube &a, const CubieCube &b) 
{
    return CubieCube(a.cp*b.cp, a.co*b.cp+b.co, a.ep*b.ep, a.eo*b.ep+b.eo);
}

inline constexpr CubieCube operator~(const CubieCube &x)
{
    auto cp_inv = ~x.cp; auto ep_inv = ~x.ep;
    return CubieCube(cp_inv, -(x.co*cp_inv), ep_inv, -(x.eo*ep_inv));
}

inline bool operator==(const CubieCube &lhs, const CubieCube &rhs) 
{ 
    return lhs.cp == rhs.cp && lhs.co == rhs.co && 
           lhs.ep == rhs.ep && lhs.eo == rhs.eo;
}

inline constexpr CornerPerm operator*(const CornerPerm &cp, const CubieCube &cc)
{
    return cp * cc.cp;
}

inline constexpr CornerOri operator*(const CornerOri &co, const CubieCube &cc)
{
    return co * cc.cp + cc.co;
}

inline constexpr EdgePerm operator*(const EdgePerm &ep, const CubieCube &cc)
{
    return ep * cc.ep;
}

inline constexpr EdgeOri operator*(const EdgeOri &eo, const CubieCube &cc)
{
    return eo * cc.ep + cc.eo;
}

/* cubiecube representations of 18 elementary moves */

inline constexpr const CubieCube mU = {{3,0,1,2,4,5,6,7},{0,0,0,0,0,0,0,0},{3,0,1,2,4,5,6,7,8,9,10,11},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr const CubieCube mR = {{4,1,2,0,7,5,6,3},{2,0,0,1,1,0,0,2},{8,1,2,3,11,5,6,7,4,9,10,0},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr const CubieCube mF = {{1,5,2,3,0,4,6,7},{1,2,0,0,2,1,0,0},{0,9,2,3,4,8,6,7,1,5,10,11},{0,1,0,0,0,1,0,0,1,1,0,0}};
inline constexpr const CubieCube mD = {{0,1,2,3,5,6,7,4},{0,0,0,0,0,0,0,0},{0,1,2,3,5,6,7,4,8,9,10,11},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr const CubieCube mL = {{0,2,6,3,4,1,5,7},{0,1,2,0,0,2,1,0},{0,1,10,3,4,5,9,7,8,2,6,11},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr const CubieCube mB = {{0,1,3,7,4,5,2,6},{0,0,1,2,0,0,2,1},{0,1,2,11,4,5,6,10,8,9,3,7},{0,0,0,1,0,0,0,1,0,0,1,1}};

inline constexpr const std::array<CubieCube,18> 
    ElementaryMove = { mU,mU*mU,mU*mU*mU,mR,mR*mR,mR*mR*mR,mF,mF*mF,mF*mF*mF,mD,mD*mD,mD*mD*mD,mL,mL*mL,mL*mL*mL,mB,mB*mB,mB*mB*mB };

inline constexpr CubieCube operator*(const CubieCube &c, const std::vector<TurnMove> &ms) 
{
    CubieCube cc = c;
    for (auto m : ms) cc = cc * ElementaryMove[m];
    return cc;
}

inline constexpr CubieCube operator*(const CubieCube &c, const std::vector<TurnAxis> &ts) 
{
    CubieCube cc = c;
    for (auto t: ts) cc = cc * ElementaryMove[t*3];
    return cc;
}