/*
 * File: cube_amalg.cpp
 * Project: cube
 * Author: coshz <fsinhx@gmail.com>
 * Version: v0.5.1
 * Date: 2026-11-08
 * Homepage: https://github.com/coshz/cube
 * License: MIT
 *
 * Copyright (c) 2026 coshz <fsinhx@gmail.com>. All rights reserved.
 */

#include <cstddef>
#include <vector>
#include <array>
#include <algorithm>
#include <utility>
#include <numeric>
#include <stdexcept>
#include <initializer_list>
#include <cassert>
#include <type_traits>
#include <cstdint>
#include <string_view>
#include <set>
#include <memory>
#include <filesystem>
#include <cstdio>
#include <limits>
#include <queue>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <tuple>
#include <cstdlib>
#include <mutex>
#include <string>
#include <cstring>
#include <regex>
#include <sstream>
#include <stdbool.h>
#include <stdint.h>

namespace cube::math {

using std::size_t;

/*!
 * @brief the cycle decompositon of permutation

 * @return pair of
    - first: vector of fixed points
    - second: vector of cycles in order of length descending
 */
template<typename VectorLike>
auto decomposite(const VectorLike &xs) -> std::pair<
    std::vector<typename VectorLike::value_type>,
    std::vector<std::vector<typename VectorLike::value_type>>
>
{
    using T = typename VectorLike::value_type;
    std::vector<T> fixed;
    std::vector<std::vector<T>> cycles;
    std::vector<bool> visited(xs.size(),false);
    for(int i = 0; i < xs.size(); ++i) {
        if(visited[i]) continue;
        std::vector<T> cycle;
        for(int j = xs[i]; j != i; j = xs[j]) {
            visited[j] = true;
            cycle.push_back(static_cast<T>(j));
        }
        visited[i] = true;
        cycle.push_back(static_cast<T>(i));
        if(cycle.size() > 1) cycles.push_back(cycle);
        else fixed.push_back(cycle[0]);
    }
    std::sort(cycles.begin(), cycles.end(), [](auto &v1, auto &v2){
        return v1.size() > v2.size();
    });
    return std::make_pair(fixed,cycles);
}

constexpr size_t factorial(size_t n)
{
    if(n>=21) throw std::invalid_argument("factorial(n) overflows for n >= 21");
    return n == 0 ? 1 : n * factorial(n-1);
}

constexpr size_t binomial(size_t n, size_t k)
{
    if(n < k)   return 0;
    if(n == k)  return 1;
    size_t r = 1, m = std::min(k, n-k);
    for(size_t i = 1; i <= m; i++) r *= n-i+1;
    for(size_t i = 1; i <= m; i++) r /= i;
    return r;
};

/* the order (period) of permutation */
template<typename T, size_t N>
size_t orderOf(const std::array<T,N> &xs)
{
    size_t m = 1;
    auto cs = decomposite(xs);
    for(size_t i = 1; i < cs.size(); i++) { m = std::lcm(m, cs[i].size()); }
    return m;
}

/* the rank of permutation */
template<typename T, size_t N>
constexpr size_t rankOf(const std::array<T,N> &xs)
{
    size_t r = 0;
    std::array<bool,N> used {false};
    for(size_t i = 0; i < N; i++)
    {
        int cnt = 0;
        for(int j=0; j < xs[i]; j++) if(!used[j]) cnt++;
        r += cnt * factorial(N - i - 1);
        used[xs[i]] = true;
    }
    return r;
}

/* create a permutation from rank */
template<typename T, size_t N>
constexpr std::array<T,N> fromRank(size_t r)
{
    std::array<T,N> A{};
    std::array<bool,N> used {false};
    for(size_t i = 0; i < N; i++)
    {
        size_t f = factorial(N - i - 1);
        size_t cnt = r / f;
        r %= f;
        for(size_t j = 0; j < N; j++)
        {
            if(used[j]) continue;
            if(cnt-- == 0) {
                A[i] = j;
                used[j] = true;
                break;
            }
        }
    }
    return A;
}

/* creat a list by applying f for N times */
template<typename F, typename T, size_t N>
constexpr std::array<T,N+1> nestList(F &&f, T &&x)
{
    std::array<T,N+1> r;
    r[0] = x;
    for(auto i = 1; i <= N; i++) r[i] = f(r[i-1]);
    return r;
}

/* take subarray of index range [Begin,End] */
template<size_t Begin, size_t End, typename T, size_t N>
constexpr auto takeByRange(const std::array<T,N>& xs) -> std::array<T,End-Begin+1>
{
    static_assert(Begin <= End && End < N);
    std::array<T,End-Begin+1> ys;
    std::copy(xs.begin()+Begin,xs.begin()+End+1,ys.begin());
    return ys;
}

/* transform: from array of digits to integer */
template<size_t B, typename Int, size_t N>
constexpr auto fromDigits(const std::array<Int,N> &xs) -> size_t
{
    auto pow = [](size_t base, size_t exp) -> size_t {
        size_t prod = 1;
        for(size_t i = 0; i < exp; i++) prod *= base;
        return prod;
    };
    size_t res = 0;
    for(size_t i = 0; i < N; i++) res += static_cast<size_t>(xs[i]) * pow(B, N-i-1);
    return res;
}

/* transform: from integer to array of digits */
template<size_t B, size_t N, typename Int>
constexpr auto toDigits(size_t i) -> std::array<Int,N>
{
    auto pow = [](size_t base, size_t exp) -> size_t {
        size_t prod = 1;
        for(size_t i = 0; i < exp; i++) prod *= base;
        return prod;
    };
    std::array<Int,N> a {};
    for(size_t k = 0; k < N; k++) {
        size_t z = pow(B, N-1-k);
        a[k] = static_cast<Int>(i / z);
        i = i % z;
    }
    return a;
}

/* the lexical order of M indices choosing from total N */
template<size_t N, size_t M>
constexpr auto lexicalOrderFromIndices(const std::array<size_t,M> &X) -> int
{
    static_assert(M<=N);
    int rank = 0;
    for(size_t i = 0; i < M; i++) rank += binomial(N-1-X[i],M-i);
    return rank;
}

/* the indices reconstructed from lexical order */
template<size_t N, size_t M>
constexpr auto lexicalOrderToIndices(int rank) -> std::array<size_t,M>
{
    static_assert(M<=N);
    std::array<size_t,M> X {};
    size_t n=rank, k=0;
    for(size_t i = 0; i < M; i++) {
        while(binomial(N-1-k,M-i) > n || n >= binomial(N-k,M-i)) k++;
        X[i] = k;
        n -= binomial(N-1-k,M-i);
        k += 1;
    }
    return X;
}

/* check the validity of permutation */
template<size_t N, typename ArrayLike>
constexpr bool isValidPermutation(const ArrayLike& xs)
{
    if(xs.size() != N) return false;
    std::array<bool,N> visited{};
    for(auto i = 0; i < N; ++i) {
        size_t val = xs[i];
        if(xs[i] < 0 || xs[i] >= N) return false;
        if(visited[val]) return false;
        visited[val] = true;
    }
    return true;
}

/*!
 * @brief Pull / Gather permute: out[i] = src[ X[i] ]
 * @note src and X must have the same dimension and X must be a permutation
 */
template<typename Array, typename Perm>
constexpr Array backpermute(const Array& src, const Perm& P)
{
    Array out{src};
    for (size_t i = 0; i < P.size(); ++i) { out[i] = src[P[i]]; }
    return out;
}

/*!
 * @brief Push / Scatter permute: out[ X[i] ] = src[i]
 * @note src and X must have the same dimension and X must be a permutation
 */
template<typename Array, typename Perm>
constexpr Array forepermute(const Array& src, const Perm& P)
{
    Array out{src};
    for (size_t i = 0; i < P.size(); ++i) { out[P[i]] = src[i]; }
    return out;
}

} // namespace cube::math
namespace cube::data {
/*
         U1 U2 U3
         U4 U5 U6
         U7 U8 U9
L1 L2 L3 F1 F2 F3 R1 R2 R3 B1 B2 B3
L4 L5 L6 F4 F5 F6 R4 R5 R6 B4 B5 B6
L7 L8 L9 F7 F8 F9 R7 R8 R9 B7 B8 B9
         D1 D2 D3
         D4 D5 D6
         D7 D8 D9
 */

enum Face       { U1,U2,U3,U4,U5,U6,U7,U8,U9,R1,R2,R3,R4,R5,R6,R7,R8,R9,F1,F2,F3,F4,F5,F6,F7,F8,F9,D1,D2,D3,D4,D5,D6,D7,D8,D9,L1,L2,L3,L4,L5,L6,L7,L8,L9,B1,B2,B3,B4,B5,B6,B7,B8,B9 };
enum Layer      { U,R,F,D,L,B };
enum TurnMove   { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
enum Symmetry   { S_URF3,S_F2,S_U4,S_LR2 };

enum ColorIndex { UCol,RCol,FCol,DCol,LCol,BCol,NoCol };
enum Corner     { URF,UFL,ULB,UBR,DFR,DLF,DBL,DRB };
enum Edge       { UR,UF,UL,UB,DR,DF,DL,DB,FR,FL,BL,BR };
struct OrientedCorner   { Corner c; unsigned o; };
struct OrientedEdge     { Edge e;   unsigned o; };

typedef ColorIndex      SingleFace[9];
typedef ColorIndex      CornerColorIndex[8][3];
typedef ColorIndex      EdgeColorIndex[12][2];
typedef ColorIndex      FaceletColor[54];
typedef Face            CentreFacelet[6];
typedef Face            CornerFacelet[8][3];
typedef Face            EdgeFacelet[12][2];
typedef Face            Facelet[54];
typedef OrientedCorner  CornerCubie[8];
typedef OrientedEdge    EdgeCubie[12];
typedef Corner          EdgeNeighbour[12][2];

const CornerColorIndex  CCI = {{UCol,RCol,FCol},{UCol,FCol,LCol},{UCol,LCol,BCol},{UCol,BCol,RCol},{DCol,FCol,RCol},{DCol,LCol,FCol},{DCol,BCol,LCol},{DCol,RCol,BCol}};
const EdgeColorIndex    ECI = {{UCol,RCol},{UCol,FCol},{UCol,LCol},{UCol,BCol},{DCol,RCol},{DCol,FCol}, {DCol,LCol},{DCol,BCol},{FCol,RCol},{FCol,LCol},{BCol,LCol},{BCol,RCol}};
const CentreFacelet     CC  = {U5,R5,F5,D5,L5,B5};
const CornerFacelet     CF  = {{U9,R1,F3},{U7,F1,L3},{U1,L1,B3},{U3,B1,R3},{D3,F9,R7},{D1,L9,F7},{D7,B9,L7},{D9,R9,B7}};
const EdgeFacelet       EF  = {{U6,R2},{U8,F2},{U4,L2},{U2,B2},{D6,R8},{D2,F8}, {D4,L8},{D8,B8},{F6,R4},{F4,L6},{B6,L4},{B4,R6}};
const EdgeNeighbour     EN  = {{URF,UBR},{UFL,URF},{ULB,UFL},{UBR,ULB},{DRB,DFR},{DFR,DLF}, {DLF,DBL},{DBL,DRB},{URF,DFR},{DLF,UFL},{DBL,ULB},{UBR,DRB}};

/*!
 * The schema of permutation p = (p[0],...,p[n-1]) (in P)
 *  i.e., a bijection on {0,...,n-1}, is called:
 *  (1) "replaced by", if: p[i] = p(i)
 *  (2) "carry to",    if: p[i] = p'(i) (p' is the inverse of p)
 * Interpretation:
 *  We expect P acts on X freely, considering the operation of p on x:
 *  (1) replace by:
 *      (X * p)[i] = X[p[i]]
 *      (p <*> q)[i] = p[q[i]]  -- (right action)
 *  (2) carry to:
 *      (p . X)[p[i]] = X[i]
 *      (q <.> p)[i] = q[p[i]]  -- (left action)
 * Symobols:
 * From above, p <*> q = p <.> q denoting p * q, right acting denoting x * p and
 * left acting denoting p * x.
 */

/* Facelet Move: using notation schema of "carry-to" */
const Facelet FaceletMove[6] = {
    {U3,U6,U9,U2,U5,U8,U1,U4,U7,F1,F2,F3,R4,R5,R6,R7,R8,R9,L1,L2,L3,F4,F5,F6,F7,F8,F9,D1,D2,D3,D4,D5,D6,D7,D8,D9,B1,B2,B3,L4,L5,L6,L7,L8,L9,R1,R2,R3,B4,B5,B6,B7,B8,B9},
    {U1,U2,B7,U4,U5,B4,U7,U8,B1,R3,R6,R9,R2,R5,R8,R1,R4,R7,F1,F2,U3,F4,F5,U6,F7,F8,U9,D1,D2,F3,D4,D5,F6,D7,D8,F9,L1,L2,L3,L4,L5,L6,L7,L8,L9,D9,B2,B3,D6,B5,B6,D3,B8,B9},
    {U1,U2,U3,U4,U5,U6,R1,R4,R7,D3,R2,R3,D2,R5,R6,D1,R8,R9,F3,F6,F9,F2,F5,F8,F1,F4,F7,L3,L6,L9,D4,D5,D6,D7,D8,D9,L1,L2,U9,L4,L5,U8,L7,L8,U7,B1,B2,B3,B4,B5,B6,B7,B8,B9},
    {U1,U2,U3,U4,U5,U6,U7,U8,U9,R1,R2,R3,R4,R5,R6,B7,B8,B9,F1,F2,F3,F4,F5,F6,R7,R8,R9,D3,D6,D9,D2,D5,D8,D1,D4,D7,L1,L2,L3,L4,L5,L6,F7,F8,F9,B1,B2,B3,B4,B5,B6,L7,L8,L9},
    {F1,U2,U3,F4,U5,U6,F7,U8,U9,R1,R2,R3,R4,R5,R6,R7,R8,R9,D1,F2,F3,D4,F5,F6,D7,F8,F9,B9,D2,D3,B6,D5,D6,B3,D8,D9,L3,L6,L9,L2,L5,L8,L1,L4,L7,B1,B2,U7,B4,B5,U4,B7,B8,U1},
    {L7,L4,L1,U4,U5,U6,U7,U8,U9,R1,R2,U1,R4,R5,U2,R7,R8,U3,F1,F2,F3,F4,F5,F6,F7,F8,F9,D1,D2,D3,D4,D5,D6,R9,R6,R3,D7,L2,L3,D8,L5,L6,D9,L8,L9,B3,B6,B9,B2,B5,B8,B1,B4,B7}
};

const Facelet FaceletSym[4] = {
    {R9,R8,R7,R6,R5,R4,R3,R2,R1,F3,F6,F9,F2,F5,F8,F1,F4,F7,U3,U6,U9,U2,U5,U8,U1,U4,U7,L1,L2,L3,L4,L5,L6,L7,L8,L9,B7,B4,B1,B8,B5,B2,B9,B6,B3,D3,D6,D9,D2,D5,D8,D1,D4,D7},
    {D9,D8,D7,D6,D5,D4,D3,D2,D1,L9,L8,L7,L6,L5,L4,L3,L2,L1,F9,F8,F7,F6,F5,F4,F3,F2,F1,U9,U8,U7,U6,U5,U4,U3,U2,U1,R9,R8,R7,R6,R5,R4,R3,R2,R1,B9,B8,B7,B6,B5,B4,B3,B2,B1},
    {U3,U6,U9,U2,U5,U8,U1,U4,U7,F1,F2,F3,F4,F5,F6,F7,F8,F9,L1,L2,L3,L4,L5,L6,L7,L8,L9,D7,D4,D1,D8,D5,D2,D9,D6,D3,B1,B2,B3,B4,B5,B6,B7,B8,B9,R1,R2,R3,R4,R5,R6,R7,R8,R9},
    {U3,U2,U1,U6,U5,U4,U9,U8,U7,L3,L2,L1,L6,L5,L4,L9,L8,L7,F3,F2,F1,F6,F5,F4,F9,F8,F7,D3,D2,D1,D6,D5,D4,D9,D8,D7,R3,R2,R1,R6,R5,R4,R9,R8,R7,B3,B2,B1,B6,B5,B4,B9,B8,B7}
};

/*!
 * @brief CornerCubie Move: using notation schema of "replace-by"
 * @remark CornerCubieMove[m][i] = {c_i,o_i} means the `i`-th corner of identity
 * cube is replaced by corner `c_i` with orientation `o_i`.
 * An orientation value `o_i` indicates the corner is rotated `o_i` times
 * clockwise from its standard orientation.
 */
const CornerCubie CornerCubieMove[6] = {
    {{UBR,0},{URF,0},{UFL,0},{ULB,0},{DFR,0},{DLF,0},{DBL,0},{DRB,0}},
    {{DFR,2},{UFL,0},{ULB,0},{URF,1},{DRB,1},{DLF,0},{DBL,0},{UBR,2}},
    {{UFL,1},{DLF,2},{ULB,0},{UBR,0},{URF,2},{DFR,1},{DBL,0},{DRB,0}},
    {{URF,0},{UFL,0},{ULB,0},{UBR,0},{DLF,0},{DBL,0},{DRB,0},{DFR,0}},
    {{URF,0},{ULB,1},{DBL,2},{UBR,0},{DFR,0},{UFL,2},{DLF,1},{DRB,0}},
    {{URF,0},{UFL,0},{UBR,1},{DRB,2},{DFR,0},{DLF,0},{ULB,2},{DBL,1}}
};

/*!
 * @brief EdgeCubieMove: using notation schema of "replace-by"
 * @remark EdgeCubieMove[m][i] = {e_i,o_i} means the `i`-th edge of identity
 * cube is replaced by edge `c_i` with orientation `o_i`.
 * An orientation value `o_i` indicates the edge is rotated clockwise (or
 * flipped) `o_i` times  from its standard orientation.
 */
const EdgeCubie EdgeCubieMove[6] = {
    {{UB,0},{UR,0},{UF,0},{UL,0},{DR,0},{DF,0},{DL,0},{DB,0},{FR,0},{FL,0},{BL,0},{BR,0}},
    {{FR,0},{UF,0},{UL,0},{UB,0},{BR,0},{DF,0},{DL,0},{DB,0},{DR,0},{FL,0},{BL,0},{UR,0}},
    {{UR,0},{FL,1},{UL,0},{UB,0},{DR,0},{FR,1},{DL,0},{DB,0},{UF,1},{DF,1},{BL,0},{BR,0}},
    {{UR,0},{UF,0},{UL,0},{UB,0},{DF,0},{DL,0},{DB,0},{DR,0},{FR,0},{FL,0},{BL,0},{BR,0}},
    {{UR,0},{UF,0},{BL,0},{UB,0},{DR,0},{DF,0},{FL,0},{DB,0},{FR,0},{UL,0},{DL,0},{BR,0}},
    {{UR,0},{UF,0},{UL,0},{BR,1},{DR,0},{DF,0},{DL,0},{BL,1},{FR,0},{FL,0},{UB,1},{DB,1}}
};

const CornerCubie CornerCubieSym[4] = {
    {{URF,1},{DFR,2},{DLF,1},{UFL,2},{UBR,2},{DRB,1},{DBL,2},{ULB,1}},
    {{DLF,0},{DFR,0},{DRB,0},{DBL,0},{UFL,0},{URF,0},{UBR,0},{ULB,0}},
    {{UBR,0},{URF,0},{UFL,0},{ULB,0},{DRB,0},{DFR,0},{DLF,0},{DBL,0}},
    {{UFL,3},{URF,3},{UBR,3},{ULB,3},{DLF,3},{DFR,3},{DRB,3},{DBL,3}}
};

const EdgeCubie EdgeCubieSym[4] = {
    {{UF,1},{FR,0},{DF,1},{FL,0},{UB,1},{BR,0},{DB,1},{BL,0},{UR,1},{DR,1},{DL,1},{UL,1}},
    {{DL,0},{DF,0},{DR,0},{DB,0},{UL,0},{UF,0},{UR,0},{UB,0},{FL,0},{FR,0},{BR,0},{BL,0}},
    {{UB,0},{UR,0},{UF,0},{UL,0},{DB,0},{DR,0},{DF,0},{DL,0},{BR,1},{FR,1},{FL,1},{BL,1}},
    {{UL,0},{UF,0},{UR,0},{UB,0},{DL,0},{DF,0},{DR,0},{DB,0},{FL,0},{FR,0},{BR,0},{BL,0}}
};

enum Constant {
    GN_HTM      = 20,       // the God's number in HTM
    GN_QTM      = 26,       // the God's number in QTM
    N_MOVE      = 18,       // 3*6, turn move
    N_TWIST     = 2187,     // 3^7, corner twist
    N_FLIP      = 2048,     // 2^11, edge flip
    N_SLICE     = 495,      // C(12,4), 4 ud-slices in correct locations, order omitted
    N_CORNER    = 40320,    // 8!, corners permutation
    N_EDGE8     = 40320,    // 8!, ud-edges permutation
    N_EDGE4     = 24,       // 4!, the order of 4 ud-slices
    N_SYM       = 48,       // cube symmetries
    N_SYM_D4h   = 16,       // D4h group (16 symmetries which preserve UD axis)
    EQ_FLIPSLICE= 64430,    // [(flip,slice):D4h], equivalent class of flipslice
    EQ_CORNER   = 2768,     // [(corner):D4h], equivalent class of corner
};
using std::size_t;
using namespace cube::math;

/*!
 * @brief Represents an permutation on the symmetric group S_n.
 *
 * @tparam N the size of the symmetric group
 * @tparam T the type of elements; T should be integral
 */
template<size_t N,typename T=int>
struct Perm
{
    using value_type = T;
    static constexpr size_t size() { return N; }

    /* default: construct a Perm identity */
    constexpr Perm() noexcept
    {
        for(size_t i = 0; i < N; ++i) X[i] = static_cast<T>(i);
    }

    /* construct from an array; throw exception if NOT a permutation */
    constexpr Perm(const std::array<T,N>& xs) noexcept
        :X{xs}
    {
        assert(isValidPermutation<N>(xs) && "array must be a valid permutation");
    }

    /* construct from a list; throw exception if NOT a permutation */
    constexpr Perm(std::initializer_list<T> xs) noexcept
    {
        assert(xs.size() == N && "size mismatch");
        auto dit = X.begin();
        // std::copy(xs.begin(), xs.end(), X.begin()); non-constexpr in C++17
        for(auto it = xs.begin(); it != xs.end(); it++, dit++) { *dit = *it; }
        assert(isValidPermutation<N>(X) && "invalid permutation list");
    }

    /*!
     * @brief the right action on ArrayLike
     *
     * @details Satisfies the right-action associativity axiom:
     *              (src * P) * Q = src * (P * Q)
     *
     *   - Vector Action: src * P   := P.backpermute(src)
     *   - Perm Product:    P * Q   := Q.backpermute(P)
     */
    template<typename ArrayLike>
    constexpr ArrayLike act_right(const ArrayLike& src) const noexcept
    {
        return backpermute(src,this->X);
    }

    friend constexpr bool operator==(const Perm &lhs, const Perm& rhs) noexcept { return lhs.X == rhs.X; }
    friend constexpr Perm operator*(const Perm &P, const Perm& Q) noexcept      { return Q.act_right(P); }

    constexpr Perm&     operator*=(const Perm& rhs) noexcept    { return *this = *this * rhs; }
    constexpr Perm      operator~() const noexcept              { return { forepermute(Perm::id.X, this->X) }; }
    constexpr T&        operator[](size_t idx) noexcept         { return X[idx]; }
    constexpr const T&  operator[](size_t idx) const noexcept   { return X[idx]; }

    /* the parity of permuation: even => true, odd => false */
    bool parity() const
    {
        const auto [fixed, cs] = decomposite(this->X);
        int s = 1;
        for(auto &c:cs) s *= (c.size() % 2 == 0) ? -1 : 1;
        return s == 1;
    }

    /* the order of a permutation */
    size_t order() const { return orderOf(X); }

    /* the rank of a permutation */
    constexpr size_t rank() const { return rankOf(X); }

    /* create Perm from rank */
    static constexpr Perm<N,T> fromRank(size_t i) { return { cube::math::fromRank<T,N>(i) }; }

    static const Perm id; // identity
    std::array<T,N> X{};
};

template<size_t N,typename T>
inline constexpr Perm<N,T> Perm<N,T>::id{};

/*!
 * @brief Represents an array on the cyclic group C_n (Z/nZ).
 *
 * @tparam N the order of C_n
 * @tparam L the length of array
 * @tparam T the integral type for elements
 */
template<size_t N, size_t L, typename T=int>
struct CArray
{
    using value_type = T;

    static constexpr size_t period = N;
    static constexpr size_t size() noexcept { return L; }

    constexpr CArray() noexcept {}

    /* construct from an array; all elements are reduced modulo N to {0,1,...,N-1} */
    constexpr CArray(const std::array<T,L> &arr) noexcept
        :xs{arr}
    {
        auto n = static_cast<T>(N);
        for(size_t i = 0; i < L; i++) { xs[i] = (xs[i] % n + n) % n; }
    }

    /* construct from a list; all elements are reduced modulo N to {0,1,...,N-1} */
    constexpr CArray(std::initializer_list<T> list) noexcept
    {
        auto n = static_cast<T>(N);
        std::size_t i = 0;
        for(T val : list) { if(i < L) { xs[i++] = (val % n + n) % n; } }
    }

    constexpr T& operator[](size_t idx) noexcept             { return xs[idx]; }
    constexpr const T& operator[](size_t idx) const noexcept { return xs[idx]; }

    constexpr CArray& operator+=(const CArray& rhs)
    {
        for(size_t i = 0; i < L; i++) xs[i] = (xs[i] + rhs.xs[i]) % N;
        return *this;
    }

    friend constexpr bool operator==(const CArray& lhs, const CArray& rhs) noexcept { return lhs.xs == rhs.xs; }

    /*!
     * @brief the operator on CArray
     * @remark result[i] = lhs[i] + rhs[i] (mod N)
     */
    friend constexpr CArray operator+(CArray lhs, const CArray& rhs) noexcept { return lhs += rhs; }

    /* the inverse */
    constexpr CArray operator~() const noexcept
    {
        CArray<N,L,T> inv {};
        auto n = static_cast<T>(N);
        for(size_t i = 0; i < L; i++) inv[i] = (n - xs[i]) % n;
        return inv;
    }

    /* the (N-modulo) sum of sequences */
    constexpr T sum() const noexcept
    {
        T s = 0;
        for(size_t i = 0; i < L; i++) s += xs[i];
        return s % N;
    }

    static const CArray id; // identity
    std::array<T,L> xs {};
};

template<size_t N, size_t L, typename T>
inline constexpr CArray<N,L,T> CArray<N,L,T>::id{};

template<size_t N, size_t L, typename T>
constexpr CArray<N,L,T> operator*(const CArray<N,L,T> &xs, const Perm<L,T> &P)
{
    return P.act_right(xs);
}

template<size_t N, size_t L, typename T>
constexpr CArray<N,L,T>& operator*=(CArray<N,L,T> &xs, const Perm<L,T> &P)
{
    return xs = P.act_right(xs);
}

template<typename T, size_t N, size_t... Ns>
struct ArrayHelper
{
    using type = std::array<typename ArrayHelper<T,Ns...>::type,N>;
};
template<typename T, size_t N>
struct ArrayHelper<T,N>
{
    using type = std::array<T,N>;
};
template<typename T, size_t... Ns>
struct NArray
{
    using value_type = T;
    static constexpr size_t size = (Ns * ...);
    static constexpr size_t dim = sizeof...(Ns);
    static constexpr std::array<size_t,dim> shape { Ns... };
    typename ArrayHelper<T,Ns...>::type storage;

    auto& operator[](size_t i) noexcept             { return storage[i]; }
    const auto& operator[](size_t i) const noexcept { return storage[i]; }

    template<typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
    auto& operator[](const std::array<U,dim>& indices) noexcept
    {
        return std::apply(
            [this](auto... args) -> decltype(auto) { return (*this)(args...); },
            indices
        );
    }
    template<typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
    const auto& operator[](const std::array<U,dim>& indices) const noexcept
    { return const_cast<NArray*>(this)->operator[](indices); }

    template<typename... Is, std::enable_if_t<sizeof...(Is)==dim, int> = 0>
    decltype(auto) operator()(Is... is) noexcept
    { return at<0>(storage, {size_t(is)...}); }

    template<typename... Is, std::enable_if_t<sizeof...(Is)==dim, int> = 0>
    decltype(auto) operator()(Is... is) const noexcept
    { return at<0>(storage, {size_t(is)...}); }

    template<typename F> void for_each(F&& f)       { for_each_rec(storage, f); }
    template<typename F> void for_each(F&& f) const { for_each_rec(storage, f); }

    void fill(const T &val) { for_each([&val](T& e){ e = val; }); }
private:
    template<size_t I, typename Arr>
    static decltype(auto) at(Arr&& arr, const std::array<size_t,dim>& indices)
    {
        if constexpr (I+1==dim) return arr[indices[I]];
        else                    return at<I+1>(arr[indices[I]], indices);
    }

    template<typename Arr, typename F>
    static void for_each_rec(Arr&& arr, F&& f)
    {
        for(auto& e: arr)
        {
            if constexpr (std::is_same_v<std::remove_reference_t<decltype(e)>,T>) {
                f(e);
            } else {
                for_each_rec(e,f);
            }
        }
    }
};

} // namespace cube::data

namespace cube {

using namespace cube::data;

typedef std::int8_t cube_value_t;
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

    Layer & operator[](size_t i) noexcept       { return s[i]; }
    Layer operator[](size_t i) const noexcept   { return s[i]; }

    // ? Should we make toFaceCube / toCubieCube constexpr

    static ColorState fromString(std::string_view cube);

    static bool is_valid_config(std::string_view cube);

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

inline constexpr CornerPerm operator*(const CornerPerm &cp, const CubieCube &cc) { return cp * cc.cp; }
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

inline constexpr FaceCube   FaceCube::id = FaceCube{eFP};
inline constexpr CubieCube  CubieCube::id = CubieCube{eCP,eCO,eEP,eEO};

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

inline constexpr std::array<TurnMove,N_MOVE> turn_entries = { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
inline constexpr std::array<TurnMove,N_MOVE> turn_inv_entries = { Ux3,Ux2,Ux1,Rx3,Rx2,Rx1,Fx3,Fx2,Fx1,Dx3,Dx2,Dx1,Lx3,Lx2,Lx1,Bx3,Bx2,Bx1 };

inline constexpr TurnMove operator~(TurnMove m) { return turn_inv_entries[m]; }

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
namespace cube {

using namespace cube::data;

ColorState ColorState::fromString(std::string_view cube)
{
    assert(is_valid_config(cube) && "invalid cube config");

    std::array<Layer,54> xs{};
    std::transform(cube.cbegin(), cube.cend(), xs.begin(), [](char c) {
        switch(c) {
        case 'U': return U;
        case 'R': return R;
        case 'F': return F;
        case 'D': return D;
        case 'L': return L;
        case 'B': return B;
        default: throw std::invalid_argument("invalid cube config");
        }
    });
    return { xs };
}

bool ColorState::is_valid_config(std::string_view cfg)
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

FaceCube ColorState::toFaceCube() const
{
    FacePerm fp{};
    for(int i = 0; i < 6; ++i) {
        fp[CC[i]] = s[CC[i]];
    }
    for(int i = 0; i < 8; i++) {
        for(int x = 0; x < 24; x++) {
            if(s[CC[CCI[i][0]]] == s[CF[x/3][x%3]]
            && s[CC[CCI[i][1]]] == s[CF[x/3][(x+1)%3]]
            && s[CC[CCI[i][2]]] == s[CF[x/3][(x+2)%3]])
            {
                fp[CF[x/3][x%3]]      = CF[i][0];
                fp[CF[x/3][(x+1)%3]]  = CF[i][1];
                fp[CF[x/3][(x+2)%3]]  = CF[i][2];
                break;
            }
        }
    }
    for(int i = 0; i < 12; i++) {
        for(int y = 0; y < 24; y++) {
            if(s[CC[ECI[i][0]]] == s[EF[y/2][y%2]]
            && s[CC[ECI[i][1]]] == s[EF[y/2][(y+1)%2]])
            {
                fp[EF[y/2][y%2]]      = EF[i][0];
                fp[EF[y/2][(y+1)%2]]  = EF[i][1];
                break;
            }
        }
    }
    return { fp };
}

CubieCube ColorState::toCubieCube() const
{
    return toFaceCube().toCubieCube();
}

CubieCube FaceCube::toCubieCube() const
{
    CubieCube cc{};
    const FaceCube &fc = *this;

    for(int i = 0; i < 8; i++) {
        for(int x = 0; x < 24; x++) {
            if(fc.f[CF[i][0]] == CF[x/3][x%3]) {
                cc.cp[i] = x/3, cc.co[i] = (3-x%3)%3;
                break;
            }
        }
    }
    for(int i = 0; i < 12; i++){
        for(int y = 0; y < 24; y++){
            if(fc.f[EF[i][0]] == EF[y/2][y%2]) {
                cc.ep[i] = y/2, cc.eo[i] = (2-y%2)%2;
                break;
            }
        }
    }
    return cc;
}

FaceCube CubieCube::toFaceCube() const
{
    FaceCube fc{};
    const CubieCube &cc = *this;

    for(int i = 0; i < 6; i++) {
        fc.f[CC[i]] = CC[i];
    }
    for(int i = 0; i < 8; i++) for(int j = 0; j < 3; j++) {
        fc.f[CF[i][j]] = CF[cc.cp[i]][(j-cc.co[i]+3)%3];
    }
    for(int i = 0; i < 12; i++) for(int j = 0; j < 2; j++) {
        fc.f[EF[i][j]] = EF[cc.ep[i]][(j-cc.eo[i]+2)%2];
    }
    return fc;
}

} // namespace cube
namespace cube::pdb {

/*!
 * @brief The coordinate space of cube.
 *
 * @details
 *  Coord, the product space of (Twist,Flip,Slice,Corner,Edge4,Edge8), is used
 * for tables generation. See CoordTables (in @ref table_coord.hh).
 *  The coordinates of:
 *  1. Ph1
 *      - twist: the orientation number of 8 corner cubies
 *      - flip: the orientation number of 12 edge cubies
 *      - slice: the location number of 4 UD-slice edge cubies (FR,FL,BL,BR)
 *  2. Ph2
 *      - corner: the location number of 8 corner cubies
 *      - edge4: the location number of 4 slice edge cubies
 *      - edge8: the location number of 8 non- UD-slice edge cubies
 *
 * @remarks
 * Alought the three components (slice2ep,edge42ep,edge82ep) of \(s,e4,e8) -> ep
 * aren't well-defined functions, the following is guaranteed:
 *  - \(m, s)-> ep2slice( slice2ep(  s ) * m ) are well-defined
 *  - \(m,e4)-> ep2edge4( edge42ep( e4 ) * m ) are well-defined if s is fixed to 0
 *  - \(m,e8)-> ep2edge8( edge82ep( e8 ) * m ) are well-deinned if s is fixed to 0
 * where m in ElementaryMove.
 */
struct Coord
{
    /*! Phase 1 coordinate
     * @param twist: the orientation number of 8 corner cubies
     * @param flip: the orientation number of 12 edge cubies
     * @param slice: the location number of 4 slice edge cubies (FR,FL,BL,BR)
     */
    int twist, flip, slice;

    /*! Phase 2 coordinate
     * @param corner: the location number of 8 corner cubies
     * @param edge4: the location number of 4 slice edge cubies
     * @param edge8: the location number of 8 non-slice edge cubies
     */
    int corner, edge4, edge8;

    static const Coord id;

    /* auxiliary conversion functions */

    static CornerOri    twist2co(int);
    static int          co2twist(const CornerOri &);

    static EdgeOri      flip2eo(int);
    static int          eo2flip(const EdgeOri &);

    static CornerPerm   corner2cp(int);
    static int          cp2corner(const CornerPerm &);

    static int          ep2slice(const EdgePerm &);
    static int          ep2edge4(const EdgePerm &);
    static int          ep2edge8(const EdgePerm &);
    static EdgePerm     slice2ep(int);        // incomplete EdgePerm
    static EdgePerm     edge42ep(int);        // incomplete EdgePerm
    static EdgePerm     edge82ep(int);        // incomplete EdgePerm
    static EdgePerm     see2ep(int,int,int);  // complete EdgePerm

    /* conversion between Coord and CubieCube */

    static Coord        CubieCube2Coord(const CubieCube &);
    static CubieCube    Coord2CubieCube(const Coord &);
};

constexpr bool operator==(const Coord &c1, const Coord &c2)
{
    return c1.twist == c2.twist && c1.flip == c2.flip && c1.slice == c2.slice
        && c1.corner == c2.corner && c1.edge4 == c2.edge4 && c1.edge8 == c2.edge8;
}

inline constexpr Coord Coord::id = { 0,0,0,0,0,0 };

/* type of TurnMove-based transformation tables  */

template<size_t coord_size>
using MoveTableLayout = NArray<uint16_t,N_MOVE,coord_size>;

using MoveTwist = MoveTableLayout<N_TWIST>;
using MoveFlip  = MoveTableLayout<N_FLIP>;
using MoveSlice = MoveTableLayout<N_SLICE>;
using MoveEdge4 = MoveTableLayout<N_EDGE4>;
using MoveEdge8 = MoveTableLayout<N_EDGE8>;
using MoveCorner= MoveTableLayout<N_CORNER>;

/**
 * @defgroup TurnMove-transformation generators
 * @{
 */

inline int transform_twist(TurnMove m, int twist)   { return Coord::co2twist(Coord::twist2co(twist) * ElementaryMove[m]); }
inline int transform_flip(TurnMove m, int flip)     { return Coord::eo2flip(Coord::flip2eo(flip) * ElementaryMove[m]); }
inline int transform_slice(TurnMove m, int slice)   { return Coord::ep2slice(Coord::slice2ep(slice) * ElementaryMove[m]); }
inline int transform_corner(TurnMove m, int corner) { return Coord::cp2corner(Coord::corner2cp(corner) * ElementaryMove[m]); }
inline int transform_edge4(TurnMove m, int edge4)   { return Coord::ep2edge4(Coord::edge42ep(edge4) * ElementaryMove[m]); }
inline int transform_edge8(TurnMove m, int edge8)   { return Coord::ep2edge8(Coord::edge82ep(edge8) * ElementaryMove[m]); }

/** @} */

} // namespace cube::pdb
namespace cube::pdb {

using namespace cube::math;
using namespace cube::data;

inline bool isSliceEdge(size_t idx)
{
    // slice edges: FR,FL,BL,BR
    return FR <= idx && idx <= BR;
}

int Coord::co2twist(const CornerOri &co)
{
    return int(fromDigits<3>(takeByRange<1,7>(co.xs)));
}

CornerOri Coord::twist2co(int i)
{
    CornerOri co;
    // only 7 components of co are independent
    auto sub1to7 = CArray<3,7,CornerOri::value_type>{toDigits<3,7,CornerOri::value_type>(i)};
    std::copy(sub1to7.xs.begin(),sub1to7.xs.end(),co.xs.begin()+1);
    co[0] = (3 - sub1to7.sum()) % 3;
    return co;
}

int Coord::eo2flip(const EdgeOri &eo)
{
    return int(fromDigits<2>(takeByRange<1,11>(eo.xs)));
}

EdgeOri Coord::flip2eo(int i)
{
    EdgeOri eo;
    // only 11 components of eo are independent
    auto sub1to11 = CArray<2,11,CornerOri::value_type>{toDigits<2,11,CornerOri::value_type>(i)};
    std::copy(sub1to11.xs.begin(),sub1to11.xs.end(),eo.xs.begin()+1);
    eo[0] = (2 - sub1to11.sum()) % 2;
    return eo;
}

int Coord::ep2slice(const EdgePerm &ep)
{
    // map: (x1,x2,x3,x4) -> C(N-1-x1,4)+C(N-1-x2,3)+C(N-1-x3,2)+C(N-1-x4,1)
    // where slice-edges indices: 0 <= x1 < x2 < x3 < x4 <= N-1;

    int x[4],slice=0,N=12;
    for(int i = 0, j = 0; i < N; i++) {
        if(isSliceEdge(ep[i])) x[j++] = i;
    }
    for(int i = 0; i < 4; i++) slice += binomial(N-1-x[i],4-i);
    return slice;
}

int Coord::ep2edge4(const EdgePerm &ep)
{
    Perm<4,EdgePerm::value_type> edge4_perm;
    for(int i = 0, j = 0; i < 4; i++, j++) {
        while(!isSliceEdge(ep[j])) j++;
        edge4_perm[i] = ep[j] - 8;
    }
    return int(edge4_perm.rank());
}

int Coord::ep2edge8(const EdgePerm &ep)
{
    Perm<8,EdgePerm::value_type> edge8_perm;
    for(size_t i = 0, j = 0; i < 8; i++, j++) {
        while(isSliceEdge(ep[j]) || ep[j] == (EdgePerm::value_type) ~0UL) j++;
        edge8_perm[i] = ep[j] - 0;
    }
    return int(edge8_perm.rank());
}

int Coord::cp2corner(const CornerPerm &cp)
{
    return int(cp.rank());
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
    for(size_t i = 0, j = 0, k = 0; i < 12; i++) {
        ep[i] = (j < 4 && i == si[j]) ? 8 + j++ : k++;
    }
    return ep;
}

EdgePerm Coord::edge42ep(int i)
{
    EdgePerm ep;
    auto e4 = Perm<4,EdgePerm::value_type>::fromRank(i);
    for(size_t i = 0; i < 12; i++) {
        ep[i] = (i < 8) ? i : e4[i-8] + 8;
    }
    return ep;
}

EdgePerm Coord::edge82ep(int i)
{
    EdgePerm ep;
    auto e8 = Perm<8,EdgePerm::value_type>::fromRank(i);
    for(size_t i = 0; i < 12; i++) {
        ep[i] = (i < 8) ? e8[i] + 0 : i;
    }
    return ep;
}

EdgePerm Coord::see2ep(int slice, int edge4, int edge8)
{
    auto e4 = Perm<4,EdgePerm::value_type>::fromRank(edge4);
    auto e8 = Perm<8,EdgePerm::value_type>::fromRank(edge8);
    auto s = lexicalOrderToIndices<12,4>(slice);
    EdgePerm ep;
    for(size_t i = 0, j = 0, x = 0, y = 0; i < 12; i++) {
        ep[i] = (j >=4 || i != s[j]) ? e8[y++]+0 : (++j,e4[x++]+8);
    }
    return ep;
}

Coord Coord::CubieCube2Coord(const CubieCube &cc)
{
    return {
        Coord::co2twist(cc.co),
        Coord::eo2flip(cc.eo),
        Coord::ep2slice(cc.ep),
        Coord::cp2corner(cc.cp),
        Coord::ep2edge4(cc.ep),
        Coord::ep2edge8(cc.ep)
    };
}

CubieCube Coord::Coord2CubieCube(const Coord &c)
{
    return {
        Coord::corner2cp(c.corner),
        Coord::twist2co(c.twist),
        Coord::see2ep(c.slice,c.edge4,c.edge8),
        Coord::flip2eo(c.flip)
    };
}

} // namespace cube::pdb
namespace cube::config {

/* set directory for move / prunning tables */
void set_table_dir(std::filesystem::path dir);

/*!
 * @brief get directory for move / prunning tables
 *
 * Resolve order:
 * - user-settings (set via @link set_table_dir)
 * - environment variable (`CUBE_TABLE_DIR`)
 * - system cache directory
 * - working directory
 */
auto get_table_dir() -> std::filesystem::path;

/* getter of `use_symmetry` */
bool use_symmetry();

/* setter of `use_symmetry` ; only the first call takes effect */
void use_symmetry(bool v);

} // namespace cube::config

namespace cube::symmetry {

using cube::CubieCube;
using namespace cube::pdb;

inline constexpr CubieCube S_U4 = {{3,0,1,2,7,4,5,6},{0,0,0,0,0,0,0,0},{3,0,1,2,7,4,5,6,11,8,9,10},{0,0,0,0,0,0,0,0,1,1,1,1}};
inline constexpr CubieCube S_RL = {{1,0,3,2,5,4,7,6},{0,0,0,0,0,0,0,0},{2,1,0,3,6,5,4,7,9,8,11,10},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr CubieCube S_F2 = {{5,4,7,6,1,0,3,2},{0,0,0,0,0,0,0,0},{6,5,4,7,2,1,0,3,9,8,11,10},{0,0,0,0,0,0,0,0,0,0,0,0}};

enum Sym: uint8_t
{
    Id, U1, U2, U3,
    RL, RL_U1, RL_U2, RL_U3,
    FF, FF_U1, FF_U2, FF_U3,
    FF_RL, FF_RL_U1, FF_RL_U2, FF_RL_U3
};

constexpr bool is_mirror(Sym s) { return (RL <= s && s <= RL_U3) || (FF_RL <= s && s <= FF_RL_U3); }

inline constexpr std::array<Sym,N_SYM_D4h> sym_entries      = { Id,U1,U2,U3,RL,RL_U1,RL_U2,RL_U3,FF,FF_U1,FF_U2,FF_U3,FF_RL,FF_RL_U1,FF_RL_U2,FF_RL_U3 };
inline constexpr std::array<Sym,N_SYM_D4h> sym_inv_entries  = { Id,U3,U2,U1,RL,RL_U1,RL_U2,RL_U3,FF,FF_U1,FF_U2,FF_U3,FF_RL,FF_RL_U3,FF_RL_U2,FF_RL_U1 };

constexpr Sym operator~(Sym s) { return sym_inv_entries[s]; }

inline constexpr std::array<CubieCube,N_SYM_D4h> SymmetricMove = {
    CubieCube::id, S_U4, S_U4*S_U4, S_U4*S_U4*S_U4,
    S_RL, S_RL*S_U4, S_RL*S_U4*S_U4, S_RL*S_U4*S_U4*S_U4,
    S_F2, S_F2*S_U4, S_F2*S_U4*S_U4, S_F2*S_U4*S_U4*S_U4,
    S_F2*S_RL, S_F2*S_RL*S_U4, S_F2*S_RL*S_U4*S_U4, S_F2*S_RL*S_U4*S_U4*S_U4
};

/* type of Sym-based conjugate tables */

typedef uint16_t                    corner_t;
typedef std::array<uint16_t,2>      flipslice_t;
typedef struct{uint16_t eq; Sym s;} corner_eq_t;
typedef struct{uint16_t eq; Sym s;} flipslice_eq_t;

template<size_t coord_size>
using ConjTableLayout = NArray<uint16_t,N_SYM_D4h,coord_size>;

using TwistConj     = ConjTableLayout<N_TWIST>;
using SliceConj     = NArray<uint16_t,N_SYM_D4h,N_SLICE>;
using Edge4Conj     = NArray<uint16_t,N_SYM_D4h,N_EDGE4>;
using Edge8Conj     = NArray<uint16_t,N_SYM_D4h,N_EDGE8>;
using CornerConj    = NArray<uint16_t,N_SYM_D4h,N_CORNER>;
using FlipSliceConj = NArray<flipslice_t,N_SYM_D4h,N_FLIP,N_SLICE>;

/**
 * @brief SymReduction, the symmetry reduction to Coord
 *
 * @details
 * Quotient space EqY := Y / Sym, defined by equivalent relation
 *  y1 ~ y2 := ∃ s ∈ Sym, y1 = s' * y2 * s (s' is the inverse of s)
 *
 * It contains:
 *  - Morphisms:
 *      [-] :: Y -> EqY, [a] := { y ∈ Y : y ~ a }
 *      rep :: EqY -> Y, rep(z) := min { y ∈ Y : [y] = z }
 *      emb :: Y -> (EqY,Sym), emb(y) := ( [y], min stab(y) )
 *      stab :: Y -> P(Sym), stab(y) := { s ∈ Sym : y = s' * rep(y) * s }
 *
 *  - Normalized transition on (X,EqY):
 *      T :: Move -> (X,EqY) -> (X,EqY)
 *           \(m,(x,c)) -> (x,c) * m := canon( (x,rep(c)) * m ),
 *      here, canon :: (X,Y) -> (X,EqY)
 *                      \(x,y) -> (s * x * s', b) where (b,s) = emb(y)
 */
struct SymReduction
{
    /* type of auxiliary tables for equivalence classes  */

    using CornerDom     = NArray<corner_eq_t,N_CORNER>;
    using EqCornerDom   = NArray<corner_t,EQ_CORNER>;
    using FlipSliceDom  = NArray<flipslice_eq_t,N_FLIP,N_SLICE>;
    using EqFlipSliceDom= NArray<flipslice_t,EQ_FLIPSLICE>;

    typedef uint16_t SymMask;
    using CornerStab    = NArray<SymMask,EQ_CORNER>;
    using FlipSliceStab = NArray<SymMask,EQ_FLIPSLICE>;
public:
    /* table-based TurnMove-transform builder */

    static auto make_trans_edge8_corner(
        const CornerDom& emb, const EqCornerDom& rep, const Edge8Conj& conj,
        const MoveEdge8& tran_edge8, const MoveCorner& tran_corner
    ) {
        return [&](TurnMove m, const std::pair<uint16_t,uint16_t>& ec) -> std::pair<uint16_t,uint16_t> {
            auto [edge8, c_corner] = ec;
            auto corner = rep(c_corner);
            auto [eq, s] = emb(tran_corner(m,corner));
            return { conj(~s,tran_edge8(m,edge8)), eq };
        };
    }

    static auto make_trans_twist_flipslice(
        const FlipSliceDom& emb, const EqFlipSliceDom& rep, const TwistConj& conj,
        const MoveTwist& tran_twist, const MoveFlip& tran_flip, const MoveSlice& tran_slice
    ) {
        return [&](TurnMove m, const std::pair<uint16_t,uint16_t>& tfs) -> std::pair<uint16_t,uint16_t> {
            auto [twist, c_flipslice] = tfs;
            auto [flip, slice] = rep(c_flipslice);
            auto [eq, s] = emb(tran_flip(m,flip),tran_slice(m,slice));
            return { conj(~s,tran_twist(m,twist)), eq };
        };
    }

    /* tables builder */

    static void build_emb_corner(CornerDom&, const CornerConj&);
    static void build_rep_corner(EqCornerDom&, const CornerDom&);
    static void build_emb_flipslice(FlipSliceDom&, const FlipSliceConj&);
    static void build_rep_flipslice(EqFlipSliceDom&, const FlipSliceDom&);
    static void build_stab_corner(CornerStab&, const EqCornerDom&, const CornerConj&);
    static void build_stab_flipslice(FlipSliceStab&, const EqFlipSliceDom&, const FlipSliceConj&);
};

/** @defgroup Sym-conjugation generators @{ */

/*!
 * @brief conjugate S' * C * S
 *  A projection from C to X comutes with Move if:
 *      proj_X (c * m ) = proj_X (c) * m, for all c in C, m in Move
 */

inline int conjugate_slice(Sym s, int slice)    { return Coord::ep2slice(SymmetricMove[~s].ep * Coord::slice2ep(slice) * SymmetricMove[s].ep); }
inline int conjugate_edge4(Sym s, int edge4)    { return Coord::ep2edge4(SymmetricMove[~s].ep * Coord::edge42ep(edge4) * SymmetricMove[s].ep); }
inline int conjugate_edge8(Sym s, int edge8)    { return Coord::ep2edge8(SymmetricMove[~s].ep * Coord::edge82ep(edge8) * SymmetricMove[s].ep); }
inline int conjugate_corner(Sym s, int corner)  { return Coord::cp2corner(SymmetricMove[~s].cp * Coord::corner2cp(corner) * SymmetricMove[s].cp); }
inline int conjugate_twist(Sym s, int twist)
{
    auto co = Coord::twist2co(twist) * SymmetricMove[s];
    return Coord::co2twist(is_mirror(s) ? ~co : co);
}
inline flipslice_t conjugate_flipslice(Sym s, const flipslice_t& fs)
{
    auto c = CubieCube { {},{},Coord::slice2ep(fs[1]),Coord::flip2eo(fs[0]) };
    c = SymmetricMove[~s] * c * SymmetricMove[s];
    return { uint16_t(Coord::eo2flip(c.eo)), uint16_t(Coord::ep2slice(c.ep)) };
}

/** @} */

} // namespace cube::symmetry
namespace cube::pdb::table {

using namespace cube::data;
using namespace cube::symmetry;

#if CUBE_ENABLE_TRACE
#   define VTRACE(...) fprintf(stderr,__VA_ARGS__)
#else
#   define VTRACE(...) do{}while(0)
#endif

/*!
 * @brief The table to cache transform of ElementaryMove on Coord
 */
template<size_t coord_size>
struct MoveTableBase: MoveTableLayout<coord_size>
{
    static constexpr size_t size = coord_size;

    /*! @param tran_fn :: Turn -> X -> X */
    template<typename F> void build(F&& tran_fn)
    {
        for(size_t x = 0; x < coord_size; ++x)
            for(auto m: turn_entries)
                (*this)(m,x) = tran_fn(m,x);
    }
};

/*!
 * @brief The table to cache transform of ElementaryMove on Coord
 */
template<size_t coord_size>
struct ConjTableBase: ConjTableLayout<coord_size>
{
    static constexpr size_t size = coord_size;

    /*! @param conj_fn ::  Sym -> X -> X */
    template<typename F> void build(F&& conj_fn)
    {
        for(size_t x = 0; x < coord_size; ++x)
            for(auto s: sym_entries)
                (*this)[s][x] = conj_fn(s,x);
    }
};

/*!
 * @brief The table to cache the distance on (Coord1,...)
 * @details
 * The (fake) distance, pt_i: Coord_i -> Nat, store the minimal count of moves
 * from id to cube such that the i-th coordinate is coord_i.
 * The following properties are useful (m is in ElementaryMove):
 *  1. pt(c) = max_i{ pt_i(c) }
 *  2. pt_i(c*m) - pt_i(c) is in {-1,0,1}
 */
template<size_t... Ns>
struct PrunTableBase: NArray<uint8_t,Ns...>
{
    using index_type = std::array<size_t,PrunTableBase::dim>;

    template<typename...MTs>
    void build_disjU(MTs&&... tables)
    {
        static_assert(sizeof...(tables) == PrunTableBase::dim);
        auto ts = std::tie(tables...);
        auto next = [ts](TurnMove m, const index_type&c) -> index_type {
            index_type c_next;
            size_t idx = 0;
            std::apply([&](auto&&... tbl){
                ( (c_next[idx] = tbl[m][c[idx]], ++idx),... );
            },ts);
            return c_next;
        };
        auto orbit = [](const index_type& c, auto&& visit) { visit(c); };
        build(next,orbit);
    }

    template<typename TranX, typename ConjX,
             typename TranY, typename EmbY, typename RepY, typename StabY>
    void build_XxEqY(TranX&& Tx, ConjX&& conj, TranY&& Ty, EmbY&& emb, RepY&& rep, StabY&& stab)
    {
        static_assert(sizeof...(Ns) == 2 && "only dim=2 is implemented");
        auto next = [&](TurnMove m, const index_type& c) -> index_type {
            auto [x, eqY] = c;
            auto y = rep(eqY);
            auto [eq, s] = emb[Ty(m,y)];
            return { conj(~s,Tx(m,x)), eq };
        };
        auto orbit = [&](const index_type& c, auto&& visit) {
            const auto [x,eqY] = c;
            for(auto s: sym_entries)
            {
                if(stab(eqY) & (1u << s)) visit(index_type{conj(s,x),eqY});
            }
        };
        build(next,orbit);
    }

    template<typename NextFn, typename OrbitFn>
    void build(NextFn&& next_fn, OrbitFn&& orbit_fn)
    {
        constexpr auto unvisited = std::numeric_limits<uint8_t>::max();
        this->fill(unvisited);
        std::queue<index_type> q;
        size_t discovered = 0, depth = 0;

        auto visit_fn = [&](const index_type& c) {
            auto& entry = (*this)[c];
            if(entry == unvisited) { entry = depth; ++discovered; }
        };

        index_type starting {};
        orbit_fn(starting,visit_fn);
        q.push(starting);
        while(!q.empty())
        {
            VTRACE("[depth %3zu] visited: %9zu / %-9zu\n",
                depth, discovered, PrunTableBase::size
            );
            ++depth;
            const auto n_depth = q.size();
            for(auto _ = 0; _ < n_depth; ++_)
            {
                auto c = q.front(); q.pop();
                for(auto m: turn_entries) {
                    const auto c_next = next_fn(m,c);
                    if((*this)[c_next] == unvisited) {
                        orbit_fn(c_next,visit_fn);
                        q.push(c_next);
                    }
                }
            }
        }
    }
};
} // namespace cube::pdb::table
#define TABLE_MAGIC 8677391316305539925
#define TABLE_VERSION 1

namespace cube::pdb::table {

namespace fs = std::filesystem;

template<typename> inline constexpr bool always_false_v = false;

enum class MoveKind { Twist, Flip, Slice, Edge4, Edge8, Corner, Count };
enum class ConjKind { TwistConj, SliceConj, Edge4Conj, Edge8Conj, CornerConj, FlipSliceConj, Count };
enum class EqKind   { RepCorner, RepFlipSlice, EmbCorner, EmbFlipSlice, Count };
enum class StabKind { CornerStab, FlipSliceStab, Count };
enum class PrunKind { SliceFlip, SliceTwist, Edge4Edge8, Edge4Corner, Edge8EqCorner, TwistEqFlipSlice, Count };

template<auto K> struct table_meta;
template<auto K> using table_t = typename table_meta<K>::type;
template<auto K> constexpr std::string_view tag_v = table_meta<K>::value;

#define Unwrap(...) __VA_ARGS__
#define spec_table_meta(Kind,Type,Tag) \
    template<> struct table_meta<Kind> \
    {                                  \
        using type = Unwrap Type;      \
        static constexpr std::string_view value = Tag; \
    };

spec_table_meta(MoveKind::Twist,        (MoveTableBase<N_TWIST>),       "TmTwist")
spec_table_meta(MoveKind::Flip,         (MoveTableBase<N_FLIP>),        "TmFlip")
spec_table_meta(MoveKind::Slice,        (MoveTableBase<N_SLICE>),       "TmSlice")
spec_table_meta(MoveKind::Edge4,        (MoveTableBase<N_EDGE4>),       "TmEdge4")
spec_table_meta(MoveKind::Edge8,        (MoveTableBase<N_EDGE8>),       "TmEdge8")
spec_table_meta(MoveKind::Corner,       (MoveTableBase<N_CORNER>),      "TmCorner")
spec_table_meta(ConjKind::TwistConj,    (ConjTableBase<N_TWIST>),       "TcTwist")
spec_table_meta(ConjKind::SliceConj,    (ConjTableBase<N_SLICE>),       "TcSlice")
spec_table_meta(ConjKind::Edge4Conj,    (ConjTableBase<N_EDGE4>),       "TcEdge4")
spec_table_meta(ConjKind::Edge8Conj,    (ConjTableBase<N_EDGE8>),       "TcEdge8")
spec_table_meta(ConjKind::CornerConj,   (ConjTableBase<N_CORNER>),      "TcCorner")
spec_table_meta(ConjKind::FlipSliceConj,(FlipSliceConj),                "TcFlipSlice")
spec_table_meta(EqKind::RepCorner,      (SymReduction::EqCornerDom),    "TeRepCorner")
spec_table_meta(EqKind::RepFlipSlice,   (SymReduction::EqFlipSliceDom), "TeRepFlipSlice")
spec_table_meta(EqKind::EmbCorner,      (SymReduction::CornerDom),      "TeEmbCorner")
spec_table_meta(EqKind::EmbFlipSlice,   (SymReduction::FlipSliceDom),   "TeEmbFlipSlice")
spec_table_meta(StabKind::CornerStab,   (SymReduction::CornerStab),     "TsCorner")
spec_table_meta(StabKind::FlipSliceStab,(SymReduction::FlipSliceStab),  "TsFlipSlice")
spec_table_meta(PrunKind::SliceFlip,        (PrunTableBase<N_SLICE,N_FLIP>),        "TpSliceFlip")
spec_table_meta(PrunKind::SliceTwist,       (PrunTableBase<N_SLICE,N_TWIST>),       "TpSliceTwist")
spec_table_meta(PrunKind::Edge4Edge8,       (PrunTableBase<N_EDGE4,N_EDGE8>),       "TpEdge4Edge8")
spec_table_meta(PrunKind::Edge4Corner,      (PrunTableBase<N_EDGE4,N_CORNER>),      "TpEdge4Corner")
spec_table_meta(PrunKind::Edge8EqCorner,    (PrunTableBase<N_EDGE8,EQ_CORNER>),     "TpEdge8EqCorner")
spec_table_meta(PrunKind::TwistEqFlipSlice, (PrunTableBase<N_TWIST,EQ_FLIPSLICE>),  "TpTwistEqFlipSlice")

#undef Unwrap
#undef spec_table_meta

template<size_t... Is> auto make_mk_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<MoveKind>(Is)>>...>;
template<size_t... Is> auto make_ck_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<ConjKind>(Is)>>...>;
template<size_t... Is> auto make_ek_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<EqKind>(Is)>>...>;
template<size_t... Is> auto make_sk_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<StabKind>(Is)>>...>;
template<size_t... Is> auto make_pk_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<PrunKind>(Is)>>...>;

using MoveTables= decltype(make_mk_tuple(std::make_index_sequence<size_t(MoveKind::Count)>{}));
using ConjTables= decltype(make_ck_tuple(std::make_index_sequence<size_t(ConjKind::Count)>{}));
using EqTables  = decltype(make_ek_tuple(std::make_index_sequence<size_t(EqKind::Count)>{}));
using StabTables= decltype(make_sk_tuple(std::make_index_sequence<size_t(StabKind::Count)>{}));
using PrunTables= decltype(make_pk_tuple(std::make_index_sequence<size_t(PrunKind::Count)>{}));
using Tables    = decltype(std::tuple_cat(
    std::declval<MoveTables>(),
    std::declval<ConjTables>(),
    std::declval<EqTables>(),
    std::declval<StabTables>(),
    std::declval<PrunTables>()
));

template<auto K>
constexpr size_t index_v = []() constexpr {
    using T = decltype(K);
    constexpr size_t C_Move = size_t(MoveKind::Count)
                    ,C_Conj = size_t(ConjKind::Count)
                    ,C_Eq   = size_t(EqKind::Count)
                    ,C_Stab = size_t(StabKind::Count);
    if constexpr (std::is_same_v<T, MoveKind>)  return size_t(K);
    if constexpr (std::is_same_v<T, ConjKind>)  return size_t(K) + C_Move;
    if constexpr (std::is_same_v<T, EqKind>)    return size_t(K) + C_Move + C_Conj;
    if constexpr (std::is_same_v<T, StabKind>)  return size_t(K) + C_Move + C_Conj + C_Eq;
    if constexpr (std::is_same_v<T, PrunKind>)  return size_t(K) + C_Move + C_Conj + C_Eq + C_Stab;
}();

class TablePool
{
    Tables tables_;
public:
    static const TablePool& instance() { return instance_mut(); }

    template<auto K> const auto& get() const
    {
        const auto& slot = std::get<index_v<K>>(tables_);
        if(!slot) {
            VTRACE("fatal: table %s is not initialized\n", tag_v<K>.data());
            std::terminate();
        }
        return *slot;
    }

    /** @param Builder :: (table_t<K>&) -> void */
    template<auto K, typename Builder> const auto& ensure(Builder&& builder)
    {
        auto& slot = std::get<index_v<K>>(tables_);
        if(!slot) {
            slot = std::make_unique<table_t<K>>();
            builder(*slot);
        }
        return *slot;
    }

    template<auto... K> void clean() { (std::get<index_v<K>>(tables_).reset(),...); }
private:
    friend class TableProvider;
    static TablePool& instance_mut() { static TablePool pool; return pool; }
};

enum class LoadStatus { Ok, Missing, Incompatible, Corrupted, OsErr };
template<auto K> struct table_with_header;

class TableProvider
{
    fs::path tdir_;
    TablePool& pool_;
public:
    template<bool use_symmetry>
    static const TableProvider& make(fs::path dir = {})
    {
        static TableProvider tp (
            dir.empty() ? cube::config::get_table_dir() : dir,
            std::bool_constant<use_symmetry>()
        );
        return tp;
    }

    template<auto K> [[nodiscard]] const auto& get() const { return pool_.get<K>(); }
private:
    template<bool use_symmetry>
    TableProvider(fs::path dir, std::bool_constant<use_symmetry>)
        : tdir_{dir}
        , pool_{TablePool::instance_mut()}
    { load_or_build<use_symmetry>(); }

    template<bool use_symmetry>
    void load_or_build()
    {
        ensure_all<
            MoveKind::Twist, MoveKind::Flip, MoveKind::Slice,
            MoveKind::Edge4, MoveKind::Edge8, MoveKind::Corner
        >();

        if constexpr (!use_symmetry) {
            ensure_all<
                PrunKind::SliceFlip, PrunKind::SliceTwist,
                PrunKind::Edge4Edge8, PrunKind::Edge4Corner
            >();
        } else {
            ensure_all<
                EqKind::EmbCorner, EqKind::EmbFlipSlice,
                ConjKind::TwistConj, ConjKind::Edge8Conj,
                PrunKind::TwistEqFlipSlice,
                PrunKind::Edge4Edge8, PrunKind::Edge8EqCorner
            >();

            pool_.clean<
                ConjKind::CornerConj, ConjKind::FlipSliceConj,
                StabKind::CornerStab, StabKind::FlipSliceStab,
                EqKind::RepCorner, EqKind::RepFlipSlice
            >();
        }
    }

    template<auto K>
    static constexpr bool should_persist()
    {
        using T = std::remove_cv_t<decltype(K)>;
        if constexpr (std::is_same_v<T, MoveKind> || std::is_same_v<T, PrunKind>) return true;
        else if constexpr (std::is_same_v<T, EqKind>) return K == EqKind::EmbCorner || K == EqKind::EmbFlipSlice;
        else if constexpr (std::is_same_v<T, ConjKind>) return K == ConjKind::TwistConj || K == ConjKind::Edge8Conj;
        else return false;
    }

    template<auto K> const auto& ensure()
    {
        return pool_.ensure<K>([this](table_t<K>& t){
            auto status = load_from<K>(t,tdir_);
            if(status == LoadStatus::Ok) return;
            if(!fs::exists(tdir_)) fs::create_directories(tdir_);
            if constexpr (std::is_same_v<decltype(K), PrunKind>) { VTRACE("[pdb] building pruning table %s...\n", tag_v<K>.data()); }
            build_table<K>(t);
            if constexpr (std::is_same_v<decltype(K), PrunKind>) { VTRACE("[pdb] done.\n"); }
            if constexpr (should_persist<K>()) save_to<K>(t,tdir_);
        });
    }

    template<auto... Ks> void ensure_all() { (ensure<Ks>(), ...); }

    /* build tables */

    template<MoveKind K> void build_table(table_t<K>& t)
    {
        if constexpr (K == MoveKind::Twist)   { t.build(transform_twist); }
        if constexpr (K == MoveKind::Flip)    { t.build(transform_flip); }
        if constexpr (K == MoveKind::Slice)   { t.build(transform_slice); }
        if constexpr (K == MoveKind::Edge4)   { t.build(transform_edge4); }
        if constexpr (K == MoveKind::Edge8)   { t.build(transform_edge8); }
        if constexpr (K == MoveKind::Corner)  { t.build(transform_corner); }
    }

    template<ConjKind K> void build_table(table_t<K>& t)
    {
        if constexpr (K == ConjKind::TwistConj)  { t.build(conjugate_twist); }
        if constexpr (K == ConjKind::SliceConj)  { t.build(conjugate_slice); }
        if constexpr (K == ConjKind::Edge4Conj)  { t.build(conjugate_edge4); }
        if constexpr (K == ConjKind::Edge8Conj)  { t.build(conjugate_edge8); }
        if constexpr (K == ConjKind::CornerConj) { t.build(conjugate_corner); }
        if constexpr (K == ConjKind::FlipSliceConj) {
            for(int flip = 0; flip < N_FLIP; ++flip) {
                for(int slice = 0; slice < N_SLICE; ++slice) {
                    for(auto s: sym_entries)
                        t[s][flip][slice] = conjugate_flipslice(s, {uint16_t(flip),uint16_t(slice)});
                }
            }
        }
    }

    template<StabKind K> void build_table(table_t<K>& t)
    {
        if constexpr(K == StabKind::CornerStab) {
            SymReduction::build_stab_corner(t,
                ensure<EqKind::RepCorner>(), ensure<ConjKind::CornerConj>()
            );
        }
        if constexpr(K == StabKind::FlipSliceStab) {
            SymReduction::build_stab_flipslice(t,
                ensure<EqKind::RepFlipSlice>(), ensure<ConjKind::FlipSliceConj>()
            );
        }
    }

    template<EqKind K> void build_table(table_t<K>& t)
    {
        if constexpr (K == EqKind::EmbCorner)   { SymReduction::build_emb_corner(t, ensure<ConjKind::CornerConj>()); }
        if constexpr (K == EqKind::RepCorner)   { SymReduction::build_rep_corner(t, ensure<EqKind::EmbCorner>()); }
        if constexpr (K == EqKind::EmbFlipSlice){ SymReduction::build_emb_flipslice(t, ensure<ConjKind::FlipSliceConj>()); }
        if constexpr (K == EqKind::RepFlipSlice){ SymReduction::build_rep_flipslice(t, ensure<EqKind::EmbFlipSlice>()); }
    }

    template<PrunKind K> void build_table(table_t<K>& t)
    {
        if constexpr (K == PrunKind::SliceTwist)    { t.build_disjU(ensure<MoveKind::Slice>(), ensure<MoveKind::Twist>()); return; }
        if constexpr (K == PrunKind::SliceFlip)     { t.build_disjU(ensure<MoveKind::Slice>(), ensure<MoveKind::Flip>()); return ; }
        if constexpr (K == PrunKind::Edge4Corner)   { t.build_disjU(ensure<MoveKind::Edge4>(), ensure<MoveKind::Corner>()); return; }
        if constexpr (K == PrunKind::Edge4Edge8)    { t.build_disjU(ensure<MoveKind::Edge4>(), ensure<MoveKind::Edge8>()); return; }

        if constexpr (K == PrunKind::Edge8EqCorner) {
            t.build_XxEqY(
                ensure<MoveKind::Edge8>(), ensure<ConjKind::Edge8Conj>(),
                ensure<MoveKind::Corner>(), ensure<EqKind::EmbCorner>(), ensure<EqKind::RepCorner>(), ensure<StabKind::CornerStab>()
            );
        }

        if constexpr (K == PrunKind::TwistEqFlipSlice) {
            const auto& tran_flip = ensure<MoveKind::Flip>();
            const auto& tran_slice = ensure<MoveKind::Slice>();
            auto tran_flipslice = [&tran_flip,&tran_slice](TurnMove m, const flipslice_t& fs) -> flipslice_t {
                return { tran_flip(m,fs[0]), tran_slice(m,fs[1]) };
            };
            t.build_XxEqY(
                ensure<MoveKind::Twist>(), ensure<ConjKind::TwistConj>(),
                tran_flipslice,
                ensure<EqKind::EmbFlipSlice>(), ensure<EqKind::RepFlipSlice>(), ensure<StabKind::FlipSliceStab>()
            );
        }
    }

    /* dump / load Tables */

    template<auto K>
    static bool save_to(const table_t<K>& table, const fs::path& dir)
    {
        static_assert(std::is_trivially_copyable_v<table_t<K>>);
        std::ofstream f(dir / tag_v<K>, std::ios::binary);
        if(!f) return false;
        return table_with_header<K>::write(f, table);
    }

    template <auto K>
    static auto load_from(table_t<K>& table, const fs::path& dir) -> LoadStatus
    {
        static_assert(std::is_trivially_copyable_v<table_t<K>>);
        std::ifstream f(dir / tag_v<K>, std::ios::binary);
        if(!f) return LoadStatus::Missing;
        return table_with_header<K>::read(f, table);
    }
};

template<auto K>
struct table_with_header
{
    struct meta_info { uint64_t filetag, checksum; };

    static constexpr auto encode_filetag(std::array<uint32_t,2> vk) -> uint64_t
    {
        const auto& [ver,kind] = vk;
        return TABLE_MAGIC ^ ((uint64_t(ver) << 32) | uint64_t(kind));
    }

    [[maybe_unused]]
    static auto decode_filetag(uint64_t tag) -> std::array<uint32_t,2>
    {
        const uint64_t vk = tag ^ TABLE_MAGIC;
        return { uint32_t(vk >> 32), uint32_t(vk & 0xffffffff) };
    }

    static constexpr uint64_t filetag_v = encode_filetag({uint32_t(TABLE_VERSION),uint32_t(index_v<K>)});

    static auto read(std::ifstream& is, table_t<K>& t) -> LoadStatus
    {
        meta_info header{};
        if(!is.read(reinterpret_cast<char*>(&header), sizeof(header))) return LoadStatus::OsErr;
        if(!(header.filetag == filetag_v)) return LoadStatus::Incompatible;
        if(!is.read(reinterpret_cast<char*>(&t.storage), sizeof(t.storage))) return LoadStatus::OsErr;
        if(fnv1a(&t.storage, sizeof(t.storage)) != header.checksum) return LoadStatus::Corrupted;
        return LoadStatus::Ok;
    }

    static auto write(std::ofstream& os, const table_t<K>& t) -> bool
    {
        const auto header = meta_info{filetag_v, fnv1a(&t.storage, sizeof(t.storage))};
        os.write(reinterpret_cast<const char*>(&header), sizeof(header));
        os.write(reinterpret_cast<const char*>(&t.storage), sizeof(t.storage));
        return bool(os);
    }

private:
    static uint64_t fnv1a(const void* data, size_t n)
    {
        const auto* p = static_cast<const uint8_t*>(data);
        uint64_t h = 1469598103934665603ull;
        for(size_t i = 0; i < n; ++i) { h ^= p[i]; h *= 1099511628211ull; }
        return h;
    }
};
} // namespace cube::pdb::table
namespace cube::pdb::ops {

using namespace cube::data;
using namespace cube::pdb::table;
using cube::config::use_symmetry;

/* The search phase */
enum Phase { Ph1=0, Ph2=1 };

struct CoordOps
{
    /**
     * @brief move-table based coord transform for phase Ph1 & Ph2
     * @attention In the current implementation, the Ph2 components of resultant
     * are left uncomputed in Ph1. Therefore, please DO NOT use this function to
     * obtain the origin of Ph2. Use @ref `ph2_origin` instead.
     */
    template<Phase PhX>
    static auto transform(const Coord&, TurnMove) -> Coord;

    /*!
     * @brief pruning-talbe based lower bound distance for phase Ph1 & Ph2
     */
    template<Phase PhX>
    static auto distance(const Coord&) -> uint8_t;

    /* the origin of PhX, from CubieCube (in Ph1) OR from applying Ph1 solution `sol` to CubieCube (in Ph2) */
    template<Phase PhX>
    static auto origin(const CubieCube& cc, const std::vector<TurnMove>& sol = {}) -> Coord;
};

template<Phase PhX>
auto CoordOps::transform(const Coord& c, TurnMove m) -> Coord
{
    const TablePool& pool = TablePool::instance();
    if constexpr (PhX == Ph1)
        return {
            pool.get<MoveKind::Twist>()(m,c.twist),
            pool.get<MoveKind::Flip>()(m,c.flip),
            pool.get<MoveKind::Slice>()(m,c.slice),
            -1,-1,-1
        };
    else
        return {
            0,0,0,
            pool.get<MoveKind::Corner>()(m,c.corner),
            pool.get<MoveKind::Edge4>()(m,c.edge4),
            pool.get<MoveKind::Edge8>()(m,c.edge8)
        };
}

template<Phase PhX>
auto CoordOps::distance(const Coord& c) -> uint8_t
{
    const TablePool& pool = TablePool::instance();

    if constexpr (PhX == Ph1) {
        if (!use_symmetry()) {
            return std::max(
                pool.get<PrunKind::SliceTwist>()(c.slice,c.twist),
                pool.get<PrunKind::SliceFlip>()(c.slice,c.flip)
            );
        } else {
            const auto [eq, s] = pool.get<EqKind::EmbFlipSlice>()(c.flip,c.slice);
            return pool.get<PrunKind::TwistEqFlipSlice>()(
                pool.get<ConjKind::TwistConj>()(~s,c.twist),
                eq
            );
        }
    } else {
        if (!use_symmetry()) {
            return std::max(
                pool.get<PrunKind::Edge4Corner>()(c.edge4,c.corner),
                pool.get<PrunKind::Edge4Edge8>()(c.edge4,c.edge8)
            );
        } else {
            const auto [eq, s] = pool.get<EqKind::EmbCorner>()(c.corner);
            return std::max(
                pool.get<PrunKind::Edge4Edge8>()(c.edge4,c.edge8),
                pool.get<PrunKind::Edge8EqCorner>()(
                    pool.get<ConjKind::Edge8Conj>()(~s,c.edge8),
                    eq
                )
            );
        }
    }
}

template<Phase PhX>
auto CoordOps::origin(const CubieCube& cc, const std::vector<TurnMove>& sol) -> Coord
{
    if constexpr (PhX == Ph1) return Coord::CubieCube2Coord(cc);
    else {
        const TablePool& pool = TablePool::instance();
        Coord c = Coord::CubieCube2Coord(cc);
        int corner = c.corner, edge4, edge8;
        auto ep = cc.ep;
        for(auto m: sol)
        {
            corner = pool.get<MoveKind::Corner>()(m,corner);
            ep = ep * ElementaryMove[m].ep;
        }
        edge4 = Coord::ep2edge4(ep);
        edge8 = Coord::ep2edge8(ep);
        return { 0,0,0,corner,edge4,edge8 };
    }
}
} // namespace cube::pdb::ops
namespace cube::solver {

using namespace cube::data;
using namespace cube::pdb;
using namespace cube::pdb::table;
using cube::pdb::ops::Phase;

class SearchContext
{
public:
    SearchContext(const CubieCube& cc = CubieCube::id, size_t good = 22)
        : root_(cc), tgtlen_(good)
    {}

    CubieCube   root() const    { return root_; }
    size_t      tgtlen() const  { return tgtlen_; }

    /* reset solution buffer */
    template<Phase PhX>
    void reset() { path_[PhX].fill(-1); }

    /* cache / fetch the move of given step */
    template<Phase PhX> void record(int step, int move){ path_[PhX][step] = move; }
    template<Phase PhX> int  recall(int step) const    { return path_[PhX][step]; }

    /* flush solution buffer to rsolution ( 0 <= L <= DS ) */
    template<Phase PhX>
    void commit(size_t L)
    {
        rsolution_[PhX].first = L;
        std::copy(path_[PhX].begin(), path_[PhX].begin()+L, rsolution_[PhX].second.begin());
    }

    /* get solution of phase 1/2 by reversing rsolution */
    template<Phase PhX>
    auto extract() const -> std::vector<TurnMove>
    {
        size_t n = rsolution_[PhX].first;
        if(n == 0) return {};
        std::vector<TurnMove> sol(n);
        for(auto i = 0; i < n; i++) {
            sol[i] = static_cast<TurnMove>(rsolution_[PhX].second[n-1-i]);
        }
        return sol;
    }

    /* max search depth for phase 1/2 (conclusion from literatures) */
    static constexpr int D0 = 12, D1 = 18, DS = D0+D1;
    template<Phase PhX> static constexpr auto D = std::get<PhX>(std::tie(D0,D1));

    /* elementary moves of two phases */
    static constexpr std::array<TurnMove,18> EM0
        = { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
    static constexpr std::array<TurnMove,10> EM1
        = { Ux1,Ux2,Ux3,Rx2,Fx2,Dx1,Dx2,Dx3,Lx2,Bx2 };
    template<Phase PhX>
    static constexpr auto& EM = std::get<PhX>(std::tie(EM0,EM1));

private:
    CubieCube root_;
    size_t tgtlen_;
    std::array<std::array<int,DS+2>,2>                  path_;      // solution buffer
    std::array<std::pair<size_t,std::array<int,DS>>,2>  rsolution_; // reverse of solution
};

/*!
 * @brief The Kociemba's algorithm to search the first solution.
 */
class KociembaSolver
{
public:
    /*!
     * @brief Two phase solver that finds the shortest solution
     * @param cc    the cube
     * @param good  early-exit threshold. This search stops as soon as a solution
     *              of length <= `good` is found. If the returned length is
     *              > `good`, the two-phase optimum is guaranteed.
     * @return (is_solved,sol1,sol2) - solX is the PhX solution. For a legal cube
     *         coord, the `is_solved` is always true.
     */
    static auto solve(const CubieCube &cc, size_t good)
        -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>;
protected:
    /* Attemp to find a solution at most `maxL` steps where the Ph1 solution to
    `c` is of exact `d` steps; Return the legnth of solution */
    static int start_search_ph1(SearchContext& ctx, const Coord& c, size_t d, size_t d_orig, size_t maxL);

    /* Attemp to find a Ph2 solution in exact `togo` steps */
    static bool start_search_ph2(SearchContext& ctx, const Coord& c, size_t togo);
};

} // namespace cube::solver

namespace cube::solver {

using namespace cube::pdb::ops;

/* important optimization
 * the continuation of TurnMoves A and its prev B could be reduced in cases:
 *  - A=Ux1,B=Ux2     (A and its prev B are "homogeneous")
 *  - A=Dx1,B=Ux1     (A > B && A and B are "disjoint")
 */
inline bool is_dull_pair(const TurnMove A, const int B)
{
    if(!(B >= Ux1 && B <= Bx3)) return false;
    return (A/3 == B/3) || (A/3 == B/3+3);
}

auto KociembaSolver::solve(const CubieCube &cc, size_t good)
    -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>
{
    size_t bestL = SearchContext::DS + 1;
    auto ctx = SearchContext(cc,good);
    ctx.reset<Ph1>();
    auto c = CoordOps::origin<Ph1>(cc);
    for(auto togo = CoordOps::distance<Ph1>(c); togo < bestL; ++togo)
    {
        int l = start_search_ph1(ctx, c,togo,togo,bestL-1);
        if(l >= 0) bestL = std::min(bestL,size_t(l));
        if(bestL <= ctx.tgtlen()) break;
    }
    if(bestL > SearchContext::DS) return {false, {}, {}};
    else return { true, ctx.extract<Ph1>(), ctx.extract<Ph2>() };
}

int KociembaSolver::start_search_ph1(
    SearchContext& ctx, const Coord& c, size_t d, size_t d_orig, size_t maxL)
{
    if(d < CoordOps::distance<Ph1>(c)) return -1;
    if(d == 0) {
        ctx.commit<Ph1>(d_orig);
        ctx.reset<Ph2>();
        auto c2 = CoordOps::origin<Ph2>(ctx.root(), ctx.extract<Ph1>());
        for(auto togo = CoordOps::distance<Ph2>(c2); togo <= maxL; ++togo)
        {
            bool succ = start_search_ph2(ctx,c2,togo);
            if(succ) {
                ctx.commit<Ph2>(togo);
                return togo;
            }
        }
        return -1;
    }

    int bestL = maxL + 1;
    for(auto m: SearchContext::EM<Ph1>)
    {
        if(is_dull_pair(m, ctx.recall<Ph1>(d))) continue;

        ctx.record<Ph1>(d-1,m);
        auto l = start_search_ph1(ctx, CoordOps::transform<Ph1>(c,m), d-1, d_orig, bestL-2);
        if(l >= 0 && bestL > l+1) {
            bestL = l+1;
        }
        if(bestL + d_orig - d <= ctx.tgtlen()) break; // current is good enough
        if(d >= bestL) break; // no shorter solution
    }
    return bestL > int(maxL) ? -1 : bestL;
}

bool KociembaSolver::start_search_ph2(
    SearchContext& ctx, const Coord& c, size_t togo)
{   if(togo < CoordOps::distance<Ph2>(c)) return false;
    if(togo == 0) return CoordOps::distance<Ph2>(c) == 0;
    for(auto m: SearchContext::EM<Ph2>)
    {
        if(is_dull_pair(m, ctx.recall<Ph2>(togo))) continue;

        ctx.record<Ph2>(togo-1,m);
        bool ret = start_search_ph2(ctx, CoordOps::transform<Ph2>(c,m), togo-1);
        if(ret) return true;
    }
    return false;
}

} // namespace cube::solver

namespace cube::config
{
namespace fs = std::filesystem;

namespace {
bool& use_symmetry_ref()
{
    static bool val_ = CUBE_USE_SYMMETRY;
    return val_;
}

fs::path& get_custom_dir_storage()
{
    static fs::path custom_dir = {};
    return custom_dir;
}

/* get the system cache directory */
auto get_cache_dir() -> std::filesystem::path
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
}

bool use_symmetry() { return use_symmetry_ref(); }
void use_symmetry(bool v)
{
    static std::once_flag once;
    std::call_once(once, [v]() { use_symmetry_ref() = v; });
}

void set_table_dir(fs::path dir)
{
    static std::once_flag once;
    std::call_once(once, [dir]() { get_custom_dir_storage() = dir; });
}

fs::path get_table_dir()
{
    // user-settings
    const auto user_dir = get_custom_dir_storage();
    if(!user_dir.empty()) return user_dir;

    // environment variable
    if(const char *env = std::getenv("CUBE_TABLE_DIR")) {
        if(env[0] != '\0') return fs::path(env);
    }

    try {
        // system cache dir
        return get_cache_dir() / "cube.coshz" / "tables";
    } catch (...) {
        return fs::absolute("tables");
    }
}

} // namespace cube::config
namespace cube::internal
{
using namespace cube::data;

/* apply maneuver to cube state */
std::string apply_maneuver(std::string_view s, const std::vector<TurnMove>& ms);

/*!
 * @brief Convert maneuver string to sequence of TurnMove.
 *
 * @remark Examples:
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
namespace cube::symmetry {

void SymReduction::build_emb_corner(CornerDom& emb, const CornerConj& conj)
{
    static_assert(EQ_CORNER <= std::numeric_limits<uint16_t>::max());

    constexpr auto unvisited = corner_eq_t { std::numeric_limits<uint16_t>::max(),Id };
    emb.fill(unvisited);
    for(int c = 0, eq = -1; c < N_CORNER; ++c)
    {
        if(emb[c].eq != unvisited.eq) continue;
        emb[c] = { uint16_t(++eq), Id };
        for(Sym s: sym_entries)
        {
            if(s == Id) continue;
            auto &entry = emb[conj(s,c)];
            if(entry.eq == unvisited.eq) entry = { uint16_t(eq), s };
        }
    }
}

void SymReduction::build_rep_corner(EqCornerDom& rep, const CornerDom& emb)
{
    const auto unvisited = std::numeric_limits<uint16_t>::max();
    rep.fill(unvisited);
    for(int corner = 0; corner < N_CORNER; ++corner)
    {
        auto eq = emb[corner].eq;
        if(rep[eq] == unvisited) rep[eq] = corner;
    }
}

void SymReduction::build_emb_flipslice(FlipSliceDom& emb, const FlipSliceConj& conj)
{
    static_assert(EQ_FLIPSLICE <= std::numeric_limits<uint16_t>::max());

    constexpr auto unvisited = flipslice_eq_t { std::numeric_limits<uint16_t>::max(),Id };
    emb.fill(unvisited);
    for(int flip = 0, eq = -1; flip < N_FLIP; ++flip)
        for(int slice = 0; slice < N_SLICE; ++slice)
    {
        if(emb(flip,slice).eq != unvisited.eq) continue;
        emb(flip,slice) = { uint16_t(++eq), Id };
        for(auto s: sym_entries)
        {
            if(s == Id) continue;
            auto fs = conj(s,flip,slice);
            auto &entry = emb(fs[0],fs[1]);
            if(entry.eq == unvisited.eq) entry = { uint16_t(eq), s };
        }
    }
}

void SymReduction::build_rep_flipslice(EqFlipSliceDom& rep, const FlipSliceDom& emb)
{
    const auto unvisited = flipslice_t { std::numeric_limits<uint16_t>::max(),std::numeric_limits<uint16_t>::max() };
    rep.fill(unvisited);
    for(int flip = 0; flip < N_FLIP; ++flip)
        for(int slice = 0; slice < N_SLICE; ++slice)
        {
            auto eq = emb(flip,slice).eq;
            if(rep[eq] == unvisited) rep[eq] = { uint16_t(flip), uint16_t(slice) };
        }
}

void SymReduction::build_stab_corner(CornerStab& stab, const EqCornerDom& rep, const CornerConj& conj)
{
    for (int eq = 0; eq < EQ_CORNER; ++eq)
    {
        SymMask mask = 0;
        const auto c = rep[eq];
        for (Sym s : sym_entries)
        {
            if (conj[s][c] == c) mask |= SymMask(1u << s);
        }
        stab[eq] = mask;
    }
}

void SymReduction::build_stab_flipslice(FlipSliceStab& stab, const EqFlipSliceDom& rep, const FlipSliceConj& conj)
{
    for (int eq = 0; eq < EQ_FLIPSLICE; ++eq)
    {
        SymMask mask = 0;
        const auto fs = rep[eq];
        for (Sym s : sym_entries)
        {
            if (conj(s,fs[0],fs[1]) == fs) mask |= SymMask(1u << s);
        }
        stab[eq] = mask;
    }
}

} // namespace cube::pdb::symmetry// SPDX-License-Identifier: MIT
// Copyright (c) 2026 coshz

#ifndef CUBE_EXPORT_H
#define CUBE_EXPORT_H

#ifdef CUBE_STATIC_DEFINE
#  define CUBE_EXPORT
#  define CUBE_NO_EXPORT
#else
#  ifndef CUBE_EXPORT
#    ifdef cube_EXPORTS
        /* We are building this library */
#      define CUBE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define CUBE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef CUBE_NO_EXPORT
#    define CUBE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef CUBE_DEPRECATED
#  define CUBE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef CUBE_DEPRECATED_EXPORT
#  define CUBE_DEPRECATED_EXPORT CUBE_EXPORT CUBE_DEPRECATED
#endif

#ifndef CUBE_DEPRECATED_NO_EXPORT
#  define CUBE_DEPRECATED_NO_EXPORT CUBE_NO_EXPORT CUBE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CUBE_NO_DEPRECATED
#    define CUBE_NO_DEPRECATED
#  endif
#endif

#endif /* CUBE_EXPORT_H */

#define CUBE_VERSION        "0.5.1"
#define CUBE_VERSION_FULL   "0.5.1"
#define CUBE_VERSION_MAJOR  0
#define CUBE_VERSION_MINOR  5
#define CUBE_VERSION_PATCH  1

/* CUBE_VERSION_CODE: a 16-bit integer encoding from (major, minor, patch) */

#define CUBE_VER_BIT_MAJOR_ 5
#define CUBE_VER_BIT_MINOR_ 7
#define CUBE_VER_BIT_PATCH_ 4

#define CUBE_VER_SFT_MAJOR_ (CUBE_VER_BIT_MINOR_ + CUBE_VER_BIT_PATCH_)
#define CUBE_VER_SFT_MINOR_ CUBE_VER_BIT_PATCH_
#define CUBE_VER_SFT_PATCH_ 0

#define CUBE_VER_MSK_MAJOR_ ((1U << CUBE_VER_BIT_MAJOR_) - 1)
#define CUBE_VER_MSK_MINOR_ ((1U << CUBE_VER_BIT_MINOR_) - 1)
#define CUBE_VER_MSK_PATCH_ ((1U << CUBE_VER_BIT_PATCH_) - 1)

#if 0 > CUBE_VERSION_MAJOR || CUBE_VERSION_MAJOR > CUBE_VER_MSK_MAJOR_
#   error "CUBE_VERSION_MAJOR out of range [0..31]"
#endif
#if 0 > CUBE_VERSION_MINOR || CUBE_VERSION_MINOR > CUBE_VER_MSK_MINOR_
#   error "CUBE_VERSION_MINOR out of range [0..127]"
#endif
#if 0 > CUBE_VERSION_PATCH || CUBE_VERSION_PATCH > CUBE_VER_MSK_PATCH_
#   error "CUBE_VERSION_PATCH out of range [0..15]"
#endif

#define CUBE_VERSION_CODE (\
    ((CUBE_VERSION_MAJOR & CUBE_VER_MSK_MAJOR_) << CUBE_VER_SFT_MAJOR_) | \
    ((CUBE_VERSION_MINOR & CUBE_VER_MSK_MINOR_) << CUBE_VER_SFT_MINOR_) | \
    ((CUBE_VERSION_PATCH & CUBE_VER_MSK_PATCH_) << CUBE_VER_SFT_PATCH_) )

#ifndef __cplusplus
#error "This is a C++ header; please use a C++ compiler OR include cube/cube.h instead"
#endif

namespace cube {

// The identity of color configuration
constexpr std::string_view cubeId = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

enum class SolveResult : int32_t {
    Success       = 0,
    Unsolvable    = 1,
    NotFound      = 2,
    InvalidSrc    = 3,
    InvalidTgt    = 4,
    UnknownErr    = 5
};

CUBE_EXPORT constexpr std::string_view to_string(SolveResult status) noexcept {
    switch (status) {
    case SolveResult::Success:      return "Success";
    case SolveResult::Unsolvable:   return "The cube configuration is unsolvable";
    case SolveResult::NotFound:     return "No solution found within the step limit";
    case SolveResult::InvalidSrc:   return "Invalid source color configuration";
    case SolveResult::InvalidTgt:   return "Invalid target color configuration";
    case SolveResult::UnknownErr:   return "Unknown error";
    }
}

enum class PermFormat : int {
    Face  = 0,
    Cubie = 1,
    Cycle = 2
};

struct Solution {
    SolveResult status;
    std::string maneuver;

    bool is_success() const noexcept
    {
        return status == SolveResult::Success;
    }
};

/*!
 * @brief Checks if a color configuration is solvable.
 */
CUBE_EXPORT bool is_solvable(std::string_view color_cube);

/*!
 * @brief Solves the Rubik's cube from source state to target state.
 * @param src,tgt   the source and target color configuration
 * @param good      early-exit threshold. This search stops as soon as a solution
 *                  of length <= `good` is found. If the returned length is
 *                  > `good`, the optimum is guaranteed.
 */
[[nodiscard]] CUBE_EXPORT Solution solve(
    std::string_view src,
    std::string_view tgt = cubeId,
    int good = 22
);

/*!
 * @brief Applies a sequence of moves to a color cube.
 * @throws std::invalid_argument if the configuration or maneuver is invalid.
 */
[[nodiscard]] CUBE_EXPORT std::string apply_maneuver(
    std::string_view maneuver,
    std::string_view cube = cubeId
);

/*!
 * @brief Computes the permutation state resulting from a maneuver or a color cube.
 * @throws std::invalid_argument if the input is invalid.
 */
[[nodiscard]] CUBE_EXPORT std::string show_permutation(
    std::string_view ms_or_cube,
    PermFormat format = PermFormat::Cycle
);

} // namespace cube// SPDX-License-Identifier: MIT
// Copyright (c) 2026 coshz

#ifndef CUBE_USE_SYMMETRY
#   define CUBE_USE_SYMMETRY 1
#endif

namespace cube {

/*!
 * @brief Initialize the cube library by loading or building lookup tables.
 * @param use_symmetry Select the table set:
 *      - `false`: smaller (<10 MB), fast to build, slower solving;
 *      - `true`:  symmetry-based (~260 MB), slower to build, faster solving.
 *      This is a memory/disk/build-time vs solve-speed trade-off.
 * @param dir Directory for reading and writting table files.
 *      Empty means the default directory (see @ref get_table_dir()).
 *      Tables are loaded if present; otherwise they are built and written here.
 * @note Only the first call takes effect; call it before the first @ref solve,
 *      which invokes it with default arguments.
 * @warning With `use_symmetry = true`, the first run may take minutes and write
 *      ~260 MB of data to @p dir.
 */
CUBE_EXPORT void cube_init(
    bool use_symmetry = CUBE_USE_SYMMETRY,
    std::string_view dir = {}
);

/* set directory for move / prunning tables */
CUBE_EXPORT void set_table_dir(std::filesystem::path dir);

/* get directory for move / prunning tables */
CUBE_EXPORT auto get_table_dir() -> std::filesystem::path;

/* check if tables exist in the table directory */
// CUBE_EXPORT bool tables_ready();

/*!
 * @brief check the validity of color configuration of cube
 * @def A configuration is valid ::= it's legal (solvable) modulo edge flips or corner twists
 */
CUBE_EXPORT bool is_valid_config(std::string_view cube);

/*!
 * @brief check the validity of maneuver (moves sequence)
 * @def A maneuver is valid ::= it's made up of moves sequence
 *   {U,U2,U',R,R2,R',F,F2,F',D,D2,D',L,L2,L',B,B2,B'} joined by spaces
 */
CUBE_EXPORT bool is_valid_maneuver(std::string_view maneuver);

} // namespace cube
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
 * @brief convert a sequence to string
 *
 * @tparam Container the type of sequence
 * @tparam Formatter the function to format each element
 *
 * @return the formatted string
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
 * @brief seq2str with default formatter that promotes int8_t / uint8_t to int
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
            return int(val);
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
               seq2str_fmt(cc.co, [](auto v) { return char('0'+v); }) +
               seq2str_fmt(cc.ep, [](auto v) { return EdgeSet[v]; }) +
               seq2str_fmt(cc.eo, [](auto v) { return char('0'+v);});
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
namespace cube {
using namespace cube::data;
using namespace cube::pdb;
using namespace cube::solver;

namespace fs = std::filesystem;

void set_table_dir(fs::path dir) { config::set_table_dir(dir); }
auto get_table_dir() -> fs::path { return config::get_table_dir(); }

bool is_valid_config(std::string_view cube) { return ColorState::is_valid_config(cube); }
bool is_valid_maneuver(std::string_view maneuver) { return internal::is_valid_maneuver(maneuver); }

void cube_init(bool use_symmetry, std::string_view dir)
{
    static std::once_flag once;
    std::call_once(once, [=]() {
        config::use_symmetry(use_symmetry);
        if(!dir.empty()) config::set_table_dir(dir);
        if(use_symmetry) table::TableProvider::make<true>();
        else table::TableProvider::make<false>();
    });
}

Solution solve(std::string_view src, std::string_view tgt, int good)
{
    if (src != cubeId && !is_valid_config(src)) return {SolveResult::InvalidSrc, ""};
    if (tgt != cubeId && !is_valid_config(tgt)) return {SolveResult::InvalidTgt, ""};

    // trivial cube
    if (src == tgt) return {SolveResult::Success, ""};

    auto cc_src = ColorState::fromString(src).toCubieCube();
    auto cc_tgt = ColorState::fromString(tgt).toCubieCube();
    CubieCube cc = ~cc_tgt * cc_src;

    // unsolvable cube
    if (!cc.isSolvable()) return {SolveResult::Unsolvable, ""};

    cube_init(CUBE_USE_SYMMETRY);

    const auto& [found, s1, s2] = KociembaSolver::solve(cc, good < 0 ? 0 : good);

    if (!found) return {SolveResult::NotFound, ""};

    // concat Ph1 & Ph2 solutions
    std::vector<TurnMove> sol;
    const size_t n1 = s1.size(), n2 = s2.size();

    // if the transition moves of ph1-ph2 are homogeneous, combine them
    if (!s1.empty() && !s2.empty() && s1[n1 - 1] / 3 == s2[0] / 3) {
        sol.insert(sol.end(), s1.begin(), s1.end() - 1);
        int m = (s1[n1 - 1] + s2[0] - s2[0] / 3 * 6 + 2) % 4;
        if (m != 0) sol.push_back(static_cast<TurnMove>(s2[0] / 3 * 3 + m - 1));
        sol.insert(sol.end(), s2.begin() + 1, s2.end());
    } else {
        sol.insert(sol.end(), s1.begin(), s1.end());
        sol.insert(sol.end(), s2.begin(), s2.end());
    }

    return {SolveResult::Success, show::to_string(sol)};
}

bool is_solvable(std::string_view color_cube) {
    return is_valid_config(color_cube) &&
           ColorState::fromString(color_cube).toCubieCube().isSolvable();
}

std::string apply_maneuver(std::string_view maneuver, std::string_view cube_state) {
    if (cube_state.size() != 54 || !is_valid_maneuver(maneuver)) {
        throw std::invalid_argument("Invalid cube format or maneuver string.");
    }
    const auto ms = internal::parse_maneuver(maneuver);
    return internal::apply_maneuver(cube_state, ms);
}

std::string show_permutation(std::string_view ms_or_cube, PermFormat format) {
    std::string cube_str;

    if (is_valid_config(ms_or_cube)) {
        cube_str = ms_or_cube;
    } else if (is_valid_maneuver(ms_or_cube)) {
        auto ms = internal::parse_maneuver(ms_or_cube);
        cube_str = internal::apply_maneuver(cubeId, ms);
    } else {
        throw std::invalid_argument("Input is neither a valid configuration nor a valid maneuver.");
    }

    auto cc = ColorState::fromString(cube_str).toCubieCube();
    return show::to_string(cc, show::CubeFormat(int(format)));
}

} // namespace cube// SPDX-License-Identifier: MIT
// Copyright (c) 2026 coshz

#ifdef __cplusplus
    #define Default(x) = x
#else
    #define Default(x)
#endif

/*******************************************************************************
 @remark Items
 Items in the cube are defined as follows:

    color       :: { U,R,F,D,L,B }
    move        :: { U,U2,U',R,R2,R',F,F2,F',D,D2,D',L,L2,L',B,B2,B' } (1~18)
    cubie       :: corner (ufl,ubr,...) OR edge (uf,ub,...)
    cube        :: the sequence of color without seperator, eg:
                   cid = `UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`
    maneuver    :: the sequence of move seperated by spaces, eg:
                   "U' F2 L R"
    permutation :: format=face      "U1U2U3...B9"
                   format=cubie     "ABCDEFGH00000000opqrstuvwxyz000000000000"
                   format=cycle     "(ufl,urf,ubr)(uf,ul,ur)(+u)(−d)"

    a color configuration is:
        solvable if: it could be obtained by moves onto the identity cube;
        valid if: it is solvable up to edge flips or corner twists.

 @remark Schema

    1. the facelet schema:

                     U1 U2 U3
                     U4 U5 U6
                     U7 U8 U9
            L1 L2 L3 F1 F2 F3 R1 R2 R3 B1 B2 B3
            L4 L5 L6 F4 F5 F6 R4 R5 R6 B4 B5 B6
            L7 L8 L9 F7 F8 F9 R7 R8 R9 B7 B8 B9
                     D1 D2 D3
                     D4 D5 D6
                     D7 D8 D9

   2. the cubie schema:

                  ^ U
                  |                            [Corners (A~H)]
             C ------- r ------- D                 A: URF    E: DFR
            /|                  /|                 B: UFL    F: DLF
           q |                 o |                 C: ULB    G: DBL
          /  y                /  z                 D: UBR    H: DRB
         B ------- p ------- A   |  ---> R
         |   |               |   |             [Edges (o~z)]
         |   G . . . v . . . | . H                 o: UR    s: DR    w: FR
         x  .                w  /                  p: UF    t: DF    x: FL
         | u                 | s                   q: UL    u: DL    y: BL
         |.                  |/                    r: UB    v: DB    z: BR
         F ------- t ------- E
        /
       v F
// ---------------------------------------------------------------------------*/

/*!
 * @brief The buffer size minimum
 * @remark
 *   - color cube    (* len=54 *);
 *   - solution      (* len<=30 OR len<=90 formated *);
 *   - permutation   (* len < 4 * L"(+urf,+ulb)" + 6 * L"(+ur,+ul)" + 6 * L"(++u)" = 4*11+6*9+6*5 = 128 *)
 * !!! the buffer size should be at least 128!!!
 */
#define CUBE_BS 128

/* the identity of color configuration */
#define CUBE_ID "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"

#ifndef CF_ENUM
    #if defined(__cplusplus)
        #define CF_ENUM(_type, _name) \
            int __CF_ENUM_##_name; \
            enum _name : _type
    #elif defined(__clang__) || defined(__OBJC__)
        #define CF_ENUM(_type, _name) \
            enum _name : _type _name; \
            enum _name: _type
    #else
        #define CF_ENUM(_type, _name) \
            _type _name; \
            enum
    #endif
#endif

typedef CF_ENUM(int32_t,SolveResult) {
    SolveResultSuccess = 0,
    SolveResultUnsolvable = 1,
    SolveResultNotFound = 2,
    SolveResultInvalidSrc = 3,
    SolveResultInvalidTgt = 4,
    SolveResultUnknownErr = 5
};

static inline const char* solve_result_info(SolveResult sr)
{
    switch(sr) {
    case SolveResultSuccess:    return "Success.";
    case SolveResultUnsolvable: return "The cube configuration is unsolvable.";
    case SolveResultNotFound:   return "No solution found within the step limit.";
    case SolveResultInvalidSrc: return "Invalid source color configuration.";
    case SolveResultInvalidTgt: return "Invalid target color configuration.";
    case SolveResultUnknownErr: return "Unknown error.";
    default:                    return "???";
    }
}

#ifdef __cplusplus
extern "C" {
#endif

/* export: solve result to string */
CUBE_EXPORT const char *solve_result_to_string(int32_t sr);

/*!
 * @brief Initialize the cube library by loading or building lookup tables.
 * @param use_symmetry Select the table set:
 *      - `false`: smaller (<10 MB), fast to build, slower solving;
 *      - `true`:  symmetry-based (~260 MB), slower to build, faster solving.
 *      This is a memory/disk/build-time vs solve-speed trade-off.
 * @param dir Directory for reading and writting table files.
 *      NULL means the default directory (see @ref get_table_dir()).
 *      Tables are loaded if present; otherwise they are built and written here.
 * @note Only the first call takes effect; call it before the first @ref solve,
 *      which invokes it with default arguments.
 * @warning With `use_symmetry = true`, the first run may take minutes and write
 *      ~260 MB of data to @p dir.
 */
CUBE_EXPORT void cube_init(
    bool use_symmetry   Default(CUBE_USE_SYMMETRY),
    const char* dir     Default(0)
);

/*!
 * @brief solve the Rubik's cube
 *
 * @param buf       the buffer to solution (space-seperated moves)
 * @param src       source color configuration, `NULL` means `id`
 * @param tgt       target color configuration, `NULL` means `id`
 * @param good      early-exit threshold. This search stops as soon as a solution
 *                  of length <= `good` is found. If the returned length is
 *                  > `good`, the optimum is guaranteed.
 * @return
 *  see enum ``SolveResult``, ``solve_result_to_string``.
 */
CUBE_EXPORT SolveResult solve(
    char*       buf,
    const char* src         Default(CUBE_ID),
    const char* tgt         Default(CUBE_ID),
    int         good        Default(22)
);

/* check the solvability of color configuration */
CUBE_EXPORT bool solvable(
    const char* color_cube  Default(CUBE_ID)
);

/**
 * @brief Applies a sequence of moves (maneuver) to a 54-character color cube.
 *
 * @param buf           output buffer to store the resulting color cube
 * @param maneuver      moves string (e.g., "R U R' U'"), `NULL` means ""
 * @param cube          color cube source, `NULL` means `CUBE_ID`
 *
 * @return
 *  false if: cube length != 54, cube_buffer is NULL, or maneuver parsing fails;
 *  true otherwise.
 */
CUBE_EXPORT bool facecube(
    char* buf,
    const char* maneuver    Default(""),
    const char *cube        Default(CUBE_ID)
);

/*!
 * @brief Computes the permutation state resulting from either a maneuver or
 *        a color cube, and outputs it in the specified format.
 *
 * @param buf         output buffer to store the resulting string
 * @param ms_or_cube  maneuver  eg., "R U R' U'"
 *                    OR cube   e.g. "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
 * @param format      output format selector (default: 2):
 *                      0 => 54-face;   eg. "U1U2U3...B9"
 *                      1 => 20-cubie;  eg. "ABCDEFGH00000000opqrstuvwxyz000000000000"
 *                      2 => cycles:    eg. "(ufl,urf,ubr)(uf,ul,ur)(+u)(−d)"
 * @return
 *  false if: perm_buffer is NULL, or maneuver parsing fails;
 *  true otherwise.
 */
CUBE_EXPORT bool permutation(
    char*       buf,
    const char* ms_or_cube  Default(CUBE_ID),
    int         format      Default(2)
);

#ifdef __cplusplus
} // extern "C"
#endif

extern "C" {

void cube_init(bool use_symmetry, const char* dir)
{
    cube::cube_init(use_symmetry, dir ? std::string_view(dir) : std::string_view{});
}

const char* solve_result_to_string(int sr)
{
    return solve_result_info(static_cast<SolveResult>(sr));
}

SolveResult solve(
    char* buf, const char *src, const char* tgt, int good)
{
    if(!buf) return SolveResultUnknownErr;
    auto s_src = src == NULL ? cube::cubeId : std::string_view(src);
    auto s_tgt = tgt == NULL ? cube::cubeId : std::string_view(tgt);
    auto res = cube::solve(s_src, s_tgt, good);
    if(res.is_success()) {
        const auto& s = res.maneuver;
        std::memcpy(buf,s.c_str(),s.size()+1);
        return SolveResultSuccess;
    }
    return static_cast<SolveResult>(res.status);
}

bool solvable(const char* cube)
{
    return cube::is_solvable(cube ? cube : cube::cubeId);
}

bool facecube(char* buf, const char *maneuver, const char *cube)
{
    if(!buf) return false;
    try {
        auto s = cube::apply_maneuver(maneuver, cube);
        std::memcpy(buf, s.c_str(), s.size()+1);
        return true;
    } catch(...) {
        buf[0] = '\0';
        return false;
    }
}

bool permutation(char* buf, const char* ms_or_cube, int format)
{
    if(!buf) return false;
    try {
        auto s = cube::show_permutation(
            ms_or_cube ? ms_or_cube : cube::cubeId,
            static_cast<cube::PermFormat>(format)
        );
        std::memcpy(buf, s.c_str(), s.size()+1);
        return true;
    } catch(...) {
        buf[0] = '\0';
        return false;
    }
}
} // extern "C"