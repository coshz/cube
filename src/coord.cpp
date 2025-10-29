#include <cmath>
#include <numeric>
#include "coord.hh"
#include "cube.hh"
#include "help.hpp"

inline bool isSliceEdge(size_t idx) 
{ 
    // slice edges: FR,FL,BL,BR
    return FR <= idx && idx <= BR;
}

int Coord::co2twist(const CornerOri &co)
{
    return static_cast<int>(fromDigits<3>(takeByRange<1,7>(co.xs)));
}

CornerOri Coord::twist2co(int i)
{
    CornerOri co;
    // only 7 components of co are independent
    auto sub1to7 = CArray<3,7,CornerOri::valut_type>{toDigits<3,7,CornerOri::valut_type>(i)};
    std::copy(sub1to7.xs.begin(),sub1to7.xs.end(),co.xs.begin()+1);
    co[0] = (3 - sub1to7.sum()) % 3;
    return co;
}

int Coord::eo2flip(const EdgeOri &eo)
{
    return static_cast<int>(fromDigits<2>(takeByRange<1,11>(eo.xs)));
}

EdgeOri Coord::flip2eo(int i)
{
    EdgeOri eo;
    // only 11 components of eo are independent
    auto sub1to11 = CArray<2,11,CornerOri::valut_type>{toDigits<2,11,CornerOri::valut_type>(i)};
    std::copy(sub1to11.xs.begin(),sub1to11.xs.end(),eo.xs.begin()+1);
    eo[0] = (2 - sub1to11.sum()) % 2;
    return eo;
}

int Coord::ep2slice(const EdgePerm &ep)
{
    // map: (x1,x2,x3,x4) -> C(N-1-x1,4)+C(N-1-x2,3)+C(N-1-x3,2)+C(N-1-x4,1)
    // where slice-edges indices: 0 <= x1 < x2 < x3 < x4 <= N-1;

    size_t x[4],slice=0,N=12;
    for(size_t i = 0, j = 0; i < N; i++) {
        if(isSliceEdge(ep[i])) x[j++] = i;
    }

    for(size_t i = 0; i < 4; i++) slice += binomial(N-1-x[i],4-i);
    return slice;
}

int Coord::ep2edge4(const EdgePerm &ep)
{
    Perm<4,EdgePerm::value_type> edge4_perm;
    for(size_t i = 0, j = 0; i < 4; i++, j++) {
        while(!isSliceEdge(ep[j])) j++;
        edge4_perm[i] = ep[j] - 8;
    }
    return edge4_perm.rank();
}

int Coord::ep2edge8(const EdgePerm &ep)
{
    Perm<8,EdgePerm::value_type> edge8_perm;
    for(size_t i = 0, j = 0; i < 8; i++, j++) {
        while(isSliceEdge(ep[j]) || ep[j] == (EdgePerm::value_type) ~0UL) j++;
        edge8_perm[i] = ep[j] - 0;
    }
    return edge8_perm.rank();
}

int Coord::cp2corner(const CornerPerm &cp)
{
    return cp.rank();
}

CornerPerm Coord::corner2cp(int i)
{
    return CornerPerm::fromRank(i);
}

EdgePerm Coord::slice2ep(int i)
{
    auto si = lexicalOrderToIndices<12,4>(i);
    EdgePerm ep;
    // placing slice edges order-independently in responding indices
    for(size_t i = 0, j = 0; i < 12; i++){
        ep[i] = (i == si[j]) ? 8 + j++ : (EdgePerm::value_type) ~0UL;
    }
    return ep;
}

EdgePerm Coord::edge42ep(int i)
{
    EdgePerm ep;
    auto e4 = Perm<4,EdgePerm::value_type>::fromRank(i);
    // placing slice edges order-dependently in normalized indices
    for(size_t i = 0; i < 12; i++){
        ep[i] = (i < 8) ? (EdgePerm::value_type) ~0UL : e4[i-8] + 8;
    }
    return ep;
}

EdgePerm Coord::edge82ep(int i)
{
    EdgePerm ep;
    auto e8 = Perm<8,EdgePerm::value_type>::fromRank(i);
    // placing non-slice edges order-dependently in normalized indices
    for(size_t i = 0; i < 12; i++){
        ep[i] = (i < 8) ? e8[i] + 0 : (EdgePerm::value_type) ~0UL;
    }
    return ep;
}

EdgePerm Coord::see2ep(int slice, int edge4, int edge8)
{
    auto e4 = Perm<4,EdgePerm::value_type>::fromRank(edge4);
    auto e8 = Perm<8,EdgePerm::value_type>::fromRank(edge8);
    auto slice_indices = lexicalOrderToIndices<12,4>(slice);
    EdgePerm ep;
    for(size_t i = 0, j = 0, x = 0, y = 0; i < 12; i++) {
        ep[i] = (i == slice_indices[j] && ++j) ? e4[x++]+8: e8[y++]+0;
    }
    return ep;
}

Coord Coord::CubieCube2Coord(const CubieCube &cc)
{
    return Coord{
        Coord::co2twist(cc.co), Coord::eo2flip(cc.eo), Coord::ep2slice(cc.ep),
        Coord::cp2corner(cc.cp), Coord::ep2edge4(cc.ep), Coord::ep2edge8(cc.ep)
    };
}

CubieCube Coord::Coord2CubieCube(const Coord &c)
{
    return CubieCube(Coord::corner2cp(c.corner), Coord::twist2co(c.twist),
                     Coord::see2ep(c.slice,c.edge4,c.edge8), Coord::flip2eo(c.flip));
}