/*
 * File: cube_amalg.cpp
 * Project: cube
 * Author: coshz <fsinhx@gmail.com>
 * Version: 0.4.0
 * Date: 2026-09-17
 * Homepage: https://github.com/coshz/cube
 * License: MIT
 *
 * Copyright (c) 2026 coshz <fsinhx@gmail.com>. All rights reserved.
 */

#include <iostream>
#include <vector>
#include <array>
#include <set>
#include <algorithm>
#include <utility>
#include <numeric>
#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <regex>
#include <string>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <optional>
#include <type_traits>
#include <string_view>
#include <cmath>
#include <fstream>
#include <limits>
#include <tuple>
#include <stdbool.h>
#include <stdint.h>
#include <format>
#include <cstring>

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
///
///////////////////////////////  Declarations  ///////////////////////////////
///

namespace cube::math {

using std::size_t;

/*!
 * \brief the cycle decompositon of permutation

 * \return pair of
    - first: vector of fixed points
    - second: vector of cycles in order of length descending
 */
template<typename VectorLike>
auto decomposite(const VectorLike &xs) -> std::pair<std::vector<typename VectorLike::value_type>,std::vector<std::vector<typename VectorLike::value_type>>>;

/* the order (period) of permutation */
template<typename T, size_t N>
size_t orderOf(const std::array<T,N> &xs);

/* the rank of permutation */
template<typename T, size_t N>
constexpr size_t rankOf(const std::array<T,N> &xs);

/* create a permutation from rank */
template<typename T, size_t N>
constexpr std::array<T,N> fromRank(size_t r);

/* creat a list by applying f for N times */
template<typename F, typename T, size_t N>
constexpr std::array<T,N+1> nestList(F &&f, T &&x);

/* take subarray of index range [Begin,End] */
template<size_t Begin, size_t End, typename T, size_t N>
constexpr auto takeByRange(const std::array<T,N>&) -> std::array<T,End-Begin+1>;

/* transform: from array of digits to integer */
template<size_t B, typename Int, size_t N>
constexpr auto fromDigits(const std::array<Int,N> &xs) -> size_t;

/* transform: from integer to array of digits */
template<size_t B, size_t N, typename Int>
constexpr auto toDigits(size_t i) -> std::array<Int,N>;

constexpr size_t factorial(size_t n);

constexpr size_t binomial(size_t n, size_t k);

/* the lexical order of M indices choosing from total N */
template<size_t N, size_t M>
constexpr auto lexicalOrderFromIndices(const std::array<size_t,M> &X) -> int;

/* the indices reconstructed from lexical order */
template<size_t N, size_t M>
constexpr auto lexicalOrderToIndices(int rank) -> std::array<size_t,M>;

/* check the validity of permutation */
template<size_t N, typename ArrayLike>
constexpr bool isValidPermutation(const ArrayLike& xs);

/*!
 * \brief Pull / Gather permute: out[i] = src[ X[i] ]
 * @note src and X must have the same dimension and X must be a permutation
 */
template<typename Array, typename Perm>
constexpr Array backpermute(const Array& src, const Perm& P);

/*!
 * \brief Push / Scatter permute: out[ X[i] ] = src[i]
 * @note src and X must have the same dimension and X must be a permutation
 */
template<typename Array, typename Perm>
constexpr Array forepermute(const Array& src, const Perm& P);

///
/////////////////////////////// Implementations ///////////////////////////////
///

template<typename VectorLike>
auto decomposite(const VectorLike &xs) -> std::pair<std::vector<typename VectorLike::value_type>,std::vector<std::vector<typename VectorLike::value_type>>>
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

template<typename T, size_t N>
size_t orderOf(const std::array<T,N> &xs)
{
    size_t m = 1;
    auto cs = decomposite(xs);
    for(size_t i = 1; i < cs.size(); i++) { m = std::lcm(m, cs[i].size()); }
    return m;
}

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

template<typename F, typename T, size_t N>
constexpr std::array<T,N+1> nestList(F &&f, T &&x)
{
    std::array<T,N+1> r;
    r[0] = x;
    for(auto i = 1; i <= N; i++) r[i] = f(r[i-1]);
    return r;
}

template<size_t Begin, size_t End, typename T, size_t N>
constexpr auto takeByRange(const std::array<T,N> &xs) -> std::array<T,End-Begin+1>
{
    static_assert(Begin <= End && End < N);
    std::array<T,End-Begin+1> ys;
    std::copy(xs.begin()+Begin,xs.begin()+End+1,ys.begin());
    return ys;
}

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

constexpr size_t binomial(size_t n, size_t k)
{
    if(n < k)   return 0;
    if(n == k)  return 1;
    size_t r = 1, m = std::min(k, n-k);
    for(size_t i = 1; i <= m; i++) r *= n-i+1;
    for(size_t i = 1; i <= m; i++) r /= i;
    return r;
};

constexpr size_t factorial(size_t n)
{
    if(n>=21) throw std::invalid_argument("factorial(n) overflows for n >= 21");
    return n == 0 ? 1 : n * factorial(n-1);
}

template<size_t N, size_t M>
constexpr auto lexicalOrderFromIndices(const std::array<size_t,M> &X) -> int
{
    static_assert(M<=N);
    int rank = 0;
    for(size_t i = 0; i < M; i++) rank += binomial(N-1-X[i],M-i);
    return rank;
}

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

template<size_t N, typename ArrayLike>
constexpr bool isValidPermutation(const ArrayLike& xs)
{
    if(xs.size() != N) return false;
    std::array<bool,N> visited{};
    for(auto i = 0; i < N; ++i) {
        size_t val = static_cast<size_t>(xs[i]);
        if(xs[i] < 0 || xs[i] >= N) return false;
        if(visited[val]) return false;
        visited[val] = true;
    }
    return true;
}

template<typename Array, typename Perm>
constexpr Array backpermute(const Array& src, const Perm& P)
{
    Array out{src};
    for (size_t i = 0; i < P.size(); ++i) { out[i] = src[P[i]]; }
    return out;
}

template<typename Array, typename Perm>
constexpr Array forepermute(const Array& src, const Perm& P)
{
    Array out{src};
    for (size_t i = 0; i < P.size(); ++i) { out[P[i]] = src[i]; }
    return out;
}

} // namespace cube::math

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

namespace cube::data {

using std::size_t;
using namespace cube::math;

/*!
 * \brief Represents an permutation on the symmetric group S_n.
 *
 * \tparam N the size of the symmetric group
 * \tparam T the type of elements; T should be integral
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
     * \brief the right action on ArrayLike
     *
     * \details Satisfies the right-action associativity axiom:
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

    friend constexpr Perm operator*(const Perm &P, const Perm& Q) noexcept
    {
        return Q.act_right(P);
    }

    constexpr Perm& operator*=(const Perm& rhs) noexcept
    {
        return *this = *this * rhs;
    }

    friend constexpr bool operator==(const Perm &lhs, const Perm& rhs) noexcept
    {
        return lhs.X == rhs.X;
    }

    /* the inverse of Perm */
    constexpr Perm operator~() const
    {
        return { forepermute(Perm::id.X, this->X) };
    }

    constexpr T& operator[](size_t idx)             { return X[idx]; }
    constexpr const T& operator[](size_t idx) const { return X[idx]; }

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
    static constexpr Perm<N,T> fromRank(size_t i)
    { return { cube::math::fromRank<T,N>(i) }; }

    static const Perm id; // identity

    std::array<T,N> X{};
};

template<size_t N,typename T>
inline constexpr Perm<N,T> Perm<N,T>::id{};

/*!
 * \brief Represents an array on the cyclic group C_n (Z/nZ).
 *
 * \tparam N the order of C_n
 * \tparam L the length of array
 * \tparam T the integral type for elements
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

    friend constexpr bool operator==(const CArray& lhs, const CArray& rhs) noexcept
    {
        return lhs.xs == rhs.xs;
    }

    constexpr CArray& operator+=(const CArray& rhs)
    {
        for(size_t i = 0; i < L; i++) xs[i] = (xs[i] + rhs.xs[i]) % N;
        return *this;
    }

    /*!
     * \brief the operator on CArray

     * \remark result[i] = lhs[i] + rhs[i] (mod N)
     */
    friend constexpr CArray operator+(CArray lhs, const CArray& rhs)
    {
        return lhs += rhs;
    }

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
struct ArrayHelper {
    using type = std::array<typename ArrayHelper<T,Ns...>::type,N>;
};
template<typename T, size_t N>
struct ArrayHelper<T,N> {
    using type = std::array<T,N>;
};
template<typename T, size_t... Ns>
struct NArray
{
    using value_type = T;
    static constexpr size_t size = (Ns * ...);
    static constexpr size_t dim = sizeof...(Ns);
    static constexpr std::array<size_t,dim> shape { Ns... };
    typename ArrayHelper<T,Ns...>::type data;
    auto & operator[](size_t i) {
        return data[i];
    }
    const auto & operator[](size_t i) const {
        return data[i];
    }
};

} // namespace cube::data

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
namespace cube {

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
 * @brief The coordinate space of cube
 * @details
 * The coordinate space Coord, the product space of six components
 * (Twist,Flip,Slice,Corner,Edge4,Edge8), is designed for two-phase algorithm;
 * see MoveTable (in @ref table.hh "table.hh") for the move transforms on Coord.
 * @implements
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
    return static_cast<int>(fromDigits<3>(takeByRange<1,7>(co.xs)));
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
    return static_cast<int>(fromDigits<2>(takeByRange<1,11>(eo.xs)));
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
    return static_cast<int>(edge4_perm.rank());
}

int Coord::ep2edge8(const EdgePerm &ep)
{
    Perm<8,EdgePerm::value_type> edge8_perm;
    for(size_t i = 0, j = 0; i < 8; i++, j++) {
        while(isSliceEdge(ep[j]) || ep[j] == (EdgePerm::value_type) ~0UL) j++;
        edge8_perm[i] = ep[j] - 0;
    }
    return static_cast<int>(edge8_perm.rank());
}

int Coord::cp2corner(const CornerPerm &cp)
{
    return static_cast<int>(cp.rank());
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

namespace cube::internal
{

/* set directory for move / prunning tables */
void set_table_dir(std::string_view dir);

/* get directory for move / prunning tables */
auto get_table_dir() -> std::filesystem::path;

/* check if tables exist in the table directory */
bool is_table_ready();

/* load tables explicitly  */
void preload_tables();

} // namespace cube::internal

namespace cube::pdb {

#define TABLE_DIR_DEFAULT cube::internal::get_table_dir()

typedef uint16_t    mt_value_t;
typedef uint8_t     pt_value_t;

/* dump / load Tables */
template <typename Table> void save_to(const Table &table, std::filesystem::path path);
template <typename Table> void load_from(Table &table, std::filesystem::path path);

template<class T>
class Singleton
{
public:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    static T& instance() {
        static T obj {};
        return obj;
    }
protected:
    Singleton() = default;
    ~Singleton() = default;
};

/*!
 * @brief The table to cache move transforms on Coord
 * @details
 * The move transform mt: Move * Coord -> Coord is a Move-action on Coord;
 * besides, the six components of Move-set Coord are still Move-sets.
 * Therefore, mt can be decomposited into the product of six components
 * mt_i: Move * Coord_i -> Coord_i; that dramatically reduces the count of
 * table items.
 * @note edge4/edge8 move tables work in phase 2 only.
 */
template<typename T=mt_value_t>
struct TableMove
{
    static_assert(std::is_integral<T>::value);
    static_assert(std::numeric_limits<T>::digits >= 16);

    using value_t = T;
    TableMove(std::filesystem::path dir = TABLE_DIR_DEFAULT);
    TableMove(const TableMove &) = delete;
    ~TableMove();
    TableMove& operator=(const TableMove &) = delete;

    template<typename Table, typename F1, typename F2>
    std::enable_if_t<Table::shape[0] == N_MOVE, void>
    buildMoveTable(Table &t, F1&& coord2i, F2&& i2coord, std::string filename="");

    /* directory to save tables */
    const std::filesystem::path tdir;

    NArray<T,N_MOVE,N_TWIST>   *pTMTwist;
    NArray<T,N_MOVE,N_FLIP>    *pTMFlip;
    NArray<T,N_MOVE,N_SLICE>   *pTMSlice;
    NArray<T,N_MOVE,N_CORNER>  *pTMCorner;
    NArray<T,N_MOVE,N_EDGE4>   *pTMEdge4;
    NArray<T,N_MOVE,N_EDGE8>   *pTMEdge8;
};

///
/// The distance on Rubik's group G
/// dist, the minimal length of maneuvers to transform from p to q.
/// dist: G -> G -> Nat,
///   dist(p,q) = dist(p*q^-1,1);
///   dist(p) = dist(p,1) = min { n: p = m1*m2*...*mn, mi in ElementaryMove },
///

/*!
 * @brief The table to cache the distance on Coord
 * @details
 * The (fake) distances, pt_i: coord_i -> Nat, store the minimal count of
 * moves from id to cube such that the i-th coord is coord_i;
 * the following properties are useful (m is in ElementaryMove):
 *  1. dist(c) >= coord_i(c);
 *  2. pt_i(c*m) - pt_i(c) is in {-1,0,1};
 */
template<typename T=pt_value_t>
struct TablePrunning
{
    using value_type = T;
    TablePrunning(std::filesystem::path dir = TABLE_DIR_DEFAULT);
    TablePrunning(const TablePrunning &) = delete;
    ~TablePrunning();
    TablePrunning operator=(const TablePrunning &) = delete;

    template<typename Table, typename MT1, typename MT2>
    std::enable_if_t<Table::shape[0] == MT1::shape[1] && Table::shape[1] == MT2::shape[1]>
    buildPrunningTable(Table &t, const MT1 &mt1, const MT2 &mt2, std::string filename);

    /* directory to save tables */
    const std::filesystem::path tdir;

    NArray<T,N_SLICE,N_FLIP>   *pTPSliceFlip;
    NArray<T,N_SLICE,N_TWIST>  *pTPSliceTwist;
    NArray<T,N_EDGE4,N_EDGE8>  *pTPEdge4Edge8;
    NArray<T,N_EDGE4,N_CORNER> *pTPEdge4Corner;
};

/* shortcut to TableMove instance */
inline const auto &get_TM() { return Singleton<TableMove<mt_value_t>>::instance(); }

/* shortcut to TablePrunning instance */
inline const auto &get_TP() { return Singleton<TablePrunning<pt_value_t>>::instance(); }

/* symmetry table
d(s^-1*x*s,1) = d(x,1), s in S.
*/

} // namespace cube::pdb
#if defined(VERBOSE) && VERBOSE
#define VPRINT(...) printf(__VA_ARGS__)
#else
#define VPRINT(...)
#endif

namespace cube::pdb {

namespace fs = std::filesystem;

template <typename Table>
void save_to(const Table &table, fs::path path)
{
    std::ofstream f(path, std::ios::binary);
    if(f.is_open()) {
        f.write(reinterpret_cast<const char*>(&table.data), sizeof(table.data));
        f.close();
    }
}

template <typename Table>
void load_from(Table &table, fs::path path)
{
    VPRINT("loading table from %s...", path.c_str());
    std::ifstream f(path,std::ios::binary);
    f.read(reinterpret_cast<char*>(&table.data), sizeof(table.data));
    f.close();
    VPRINT("done.\n");
}

template<typename T>
template<typename Table, typename F1, typename F2>
std::enable_if_t<Table::shape[0] == N_MOVE>
TableMove<T>::buildMoveTable(Table &t, F1&& coord2i, F2&& i2coord, std::string filename)
{
    VPRINT("creating move table %s of shape (%zu,%zu)... ",
           filename.c_str(), t.shape[0], t.shape[1]);
    for(size_t i = 0; i < t.shape[0]; i++) for(size_t j = 0; j < t.shape[1]; j++) {
        t[i][j] = coord2i(i2coord(j) * ElementaryMove[i]);
    }
    if(filename != "") save_to(t, tdir/filename);
    VPRINT("done.\n");
}

template<typename T>
TableMove<T>::TableMove(std::filesystem::path dir)
:tdir(dir)
{
    VPRINT("INIT MOVE TABLES -- \n");
    pTMTwist     = new NArray<T,N_MOVE,N_TWIST>;
    pTMFlip      = new NArray<T,N_MOVE,N_FLIP>;
    pTMSlice     = new NArray<T,N_MOVE,N_SLICE>;
    pTMCorner    = new NArray<T,N_MOVE,N_CORNER>;
    pTMEdge4     = new NArray<T,N_MOVE,N_EDGE4>;
    pTMEdge8     = new NArray<T,N_MOVE,N_EDGE8>;

    if(!fs::exists(tdir/"tm_twist.dat")) {
        if(!fs::exists(tdir)) fs::create_directories(tdir);
        buildMoveTable(*pTMTwist, Coord::co2twist, Coord::twist2co, "tm_twist.dat");
        buildMoveTable(*pTMFlip, Coord::eo2flip, Coord::flip2eo, "tm_flip.dat");
        buildMoveTable(*pTMSlice, Coord::ep2slice, Coord::slice2ep, "tm_slice.dat");
        buildMoveTable(*pTMCorner, Coord::cp2corner, Coord::corner2cp, "tm_corner.dat");
        buildMoveTable(*pTMEdge4, Coord::ep2edge4, Coord::edge42ep, "tm_edge4.dat");
        buildMoveTable(*pTMEdge8, Coord::ep2edge8, Coord::edge82ep, "tm_edge8.dat");
    } else {
        load_from(*pTMTwist, tdir/"tm_twist.dat");
        load_from(*pTMFlip, tdir/"tm_flip.dat");
        load_from(*pTMSlice, tdir/"tm_slice.dat");
        load_from(*pTMCorner, tdir/"tm_corner.dat");
        load_from(*pTMEdge4, tdir/"tm_edge4.dat");
        load_from(*pTMEdge8, tdir/"tm_edge8.dat");
    }
    VPRINT("-- DONE.\n");
}

template<typename T>
TableMove<T>::~TableMove()
{
    delete pTMTwist;
    delete pTMFlip;
    delete pTMSlice;
    delete pTMCorner;
    delete pTMEdge4;
    delete pTMEdge8;
}

template<typename T>
template<typename Table, typename MT1, typename MT2>
std::enable_if_t<Table::shape[0] == MT1::shape[1] && Table::shape[1] == MT2::shape[1]>
TablePrunning<T>::buildPrunningTable(
    Table &t, const MT1 &mt1, const MT2 &mt2, std::string filename)
{
    VPRINT("creating prunning table %s of shape (%zu,%zu):\n",
           filename.c_str(), mt1.shape[1], mt2.shape[1]);
    std::fill_n(&t.data[0][0], t.size, (typename Table::value_type) ~0UL);
    t[0][0] = 0;

    typename Table::value_type depth = 0;
    size_t count = 1;
    VPRINT("\tdepth %2d: %10zu / %-10zu.\n", depth, count, t.size);
    while(count < t.size)
    {
        for(size_t i = 0; i < t.shape[0]; i++)
        for(size_t j = 0; j < t.shape[1]; j++)
        if(t[i][j] == depth) {
            for(auto k = 0; k < N_MOVE; k++) {
                auto ii = mt1[k][i], jj = mt2[k][j];
                if(t[ii][jj] == (typename Table::value_type)~0UL) { t[ii][jj] = depth + 1; count++; }
            }
        }
        depth++;
        VPRINT("\tdepth %2d: %10zu / %-10zu.\n", depth, count, t.size);
    }
    if(filename != "") save_to(t, tdir/filename);
    VPRINT("done.\n");
}

template<typename T>
TablePrunning<T>::TablePrunning(std::filesystem::path dir)
:tdir(dir)
{
    VPRINT("INIT PRUNNING TABLES -- \n");
    pTPSliceFlip     = new NArray<T,N_SLICE,N_FLIP>;
    pTPSliceTwist    = new NArray<T,N_SLICE,N_TWIST>;
    pTPEdge4Edge8    = new NArray<T,N_EDGE4,N_EDGE8>;
    pTPEdge4Corner   = new NArray<T,N_EDGE4,N_CORNER>;

    if(!fs::exists(tdir/"tp_slicetwist.dat")) {
        const auto &TM = get_TM();
        buildPrunningTable(*pTPSliceTwist, *TM.pTMSlice, *TM.pTMTwist, "tp_slicetwist.dat");
        buildPrunningTable(*pTPSliceFlip, *TM.pTMSlice, *TM.pTMFlip, "tp_sliceflip.dat");
        buildPrunningTable(*pTPEdge4Corner, *TM.pTMEdge4, *TM.pTMCorner, "tp_edge4corner.dat");
        buildPrunningTable(*pTPEdge4Edge8, *TM.pTMEdge4, *TM.pTMEdge8, "tp_edge4edge8.dat");
    } else {
        load_from(*pTPSliceTwist, tdir/"tp_slicetwist.dat");
        load_from(*pTPSliceFlip, tdir/"tp_sliceflip.dat");
        load_from(*pTPEdge4Corner,tdir/"tp_edge4corner.dat");
        load_from(*pTPEdge4Edge8, tdir/"tp_edge4edge8.dat");
    }
    VPRINT("-- DONE.\n");
}

template<typename T>
TablePrunning<T>::~TablePrunning()
{
    delete pTPSliceFlip;
    delete pTPSliceTwist;
    delete pTPEdge4Edge8;
    delete pTPEdge4Corner;
}

template struct TableMove<>;
template struct TablePrunning<>;

} // namespace pdb

namespace cube::solver {
using namespace cube::pdb;

/*!
 * @brief The Kociemba's twophase algorithm
 */
class TwoPhaseSolver
{
public:

    /*!
     * @brief Attempt to solve `c` in `step` steps
     * @param c the Coord of cube
     * @param step the maximum steps allowed for search
     * @param best whether to search exhaustively for the shortest solution
     * @return (is_solved,sol1,sol2) - solN means solution in phase N
     * @note
     * We adopt two iterative deepening searches in series rather than nesting
     * them, as in the original Kociemba algorithm. This simplifies the
     * implementation, but sacrifices the ease of extending the algorithm to
     * find the two-phase optimum. (Btw, the two-phase optimal solution is
     * intrinsically not guaranteed to be theoretically optimal.)
     */
    auto solve(const Coord &c, int step, bool best)
        -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>;

protected:
    enum enum_phase { Ph1=0, Ph2=1 };

    /*!
     * @brief The search algorithm in PhX
     * @param c the Coord of cube
     * @param togo the maximum search depth
     * @return `true` if a solution is found within depth `togo`
     *
     * @details The search tree is rooted at coord `c` and expands via ElementaryMove,
     * i.e., the `m`-th child of node `c` is `c * ElementaryMove[m]`.
     * During DFS traversal, the current move is always cached in buffer `sofar_`
     * at index `togo-1`. If a solution node is reached, the buffer is flushed
     * into `rsolution_` and return true. Otherwise, it returns false after all
     * nodes up to depth `togo` have been explored.
     * A node `x` is considered a solution if `distance<PHX>(x) == 0`.
     */
    template<enum_phase PhX> bool search_phase(const Coord &c, size_t togo);

    /* move-table based coord transform */
    template<enum_phase PhX> static Coord transform(const Coord &c, const TurnMove &m);

    /* prunning-talbe based lower bound distance in phase 1/2 */
    template<enum_phase PhX> static size_t distance(const Coord &c);

    /* max search depth for phase 1/2 (conclusion from literatures) */
    static constexpr int D0 = 12, D1 = 18, DS = D0+D1;
    template<enum_phase PhX> static constexpr auto& D = std::get<PhX>(std::tie(D0,D1));

    /* elementary moves of two phases */
    static constexpr std::array<TurnMove,18> EM0
        = { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
    static constexpr std::array<TurnMove,10> EM1
        = { Ux1,Ux2,Ux3,Rx2,Fx2,Dx1,Dx2,Dx3,Lx2,Bx2 };
    template<enum_phase PhX>
    static constexpr auto& EM = std::get<PhX>(std::tie(EM0,EM1));

private:

    /* reset solution buffer */
    template<enum_phase PhX>
    void reset_ph_sofar_()  { sofar_[PhX].fill(-1); }

    /* flush solution buffer to rsolution ( 0 <= L <= DS ) */
    template<enum_phase PhX>
    void set_ph_solution_(size_t L)
    {
        rsolution_[PhX].first = L;
        std::copy(sofar_[PhX].begin(), sofar_[PhX].begin()+L, rsolution_[PhX].second.begin());
    }

    /* get solution of phase 1/2 by reversing rsolution */
    template<enum_phase PhX>
    auto get_ph_solution_() const -> std::vector<TurnMove>
    {
        size_t n = rsolution_[PhX].first;
        if(n == 0) return {};
        std::vector<TurnMove> sol(n);
        for(auto i = 0; i < n; i++) {
            sol[i] = static_cast<TurnMove>(rsolution_[PhX].second[n-1-i]);
        }
        return sol;
    }

    /* the origin of phase 2, evaluated from phase 1 solution */
    Coord ph2_origin_(Coord c) const;

    std::array<std::array<int,DS+2>,2>                  sofar_;      // solution buffer
    std::array<std::pair<size_t,std::array<int,DS>>,2>  rsolution_;  // reverse of solution
};

} // namespace cube::solver
#define TM get_TM()
#define TP get_TP()

namespace cube::solver {

/* for optimization
 * the continuation of TurnMoves A,B,C are dull (could be reduced) in cases like:
 *  - A=Ux1,B=Ux2,C     (A and its prev B are "homogeneous")
 *  - A=Ux1,B=Dx1,C=Ux1 (A and B are "disjoint" && A and C are "homogeneous")
 */
inline bool is_dull_triple(const TurnMove A, const int B, const int C)
{
    return ( B>=Ux1 && B<=Bx3 ) &&
           ( (A/3==B/3) || ((C>=Ux1&&C<=Bx3) && A/3==C/3 && (3+A/3-B/3)%3==0) )
    ;
}

template<TwoPhaseSolver::enum_phase I>
Coord TwoPhaseSolver::transform(const Coord &c, const TurnMove &m)
{
    if constexpr (I == Ph1)
    return Coord {
        (*TM.pTMTwist)[m][c.twist], (*TM.pTMFlip)[m][c.flip], (*TM.pTMSlice)[m][c.slice],
        -1,-1,-1 /* -1: not used */
    };
    else
    return Coord {
        0,0,0,
        (*TM.pTMCorner)[m][c.corner], (*TM.pTMEdge4)[m][c.edge4], (*TM.pTMEdge8)[m][c.edge8]
    };
}

template<TwoPhaseSolver::enum_phase I>
size_t TwoPhaseSolver::distance(const Coord &c)
{
    if constexpr (I == Ph1)
    return std::max((*TP.pTPSliceTwist)[c.slice][c.twist],
                    (*TP.pTPSliceFlip)[c.slice][c.flip]);
    else
    return std::max((*TP.pTPEdge4Corner)[c.edge4][c.corner],
                    (*TP.pTPEdge4Edge8)[c.edge4][c.edge8]);
}

template<TwoPhaseSolver::enum_phase PhX>
bool TwoPhaseSolver::search_phase(const Coord &c, size_t togo)
{
    if(togo == 0) return distance<PhX>(c) == 0;
    if(togo < distance<PhX>(c)) return false;

    for(auto m: EM<PhX>)
    {
        // assert(togo+1 < D);
        if(is_dull_triple(m,sofar_[PhX][togo],sofar_[PhX][togo+1])) continue;

        sofar_[PhX][togo-1] = m;
        bool ret = search_phase<PhX>(transform<PhX>(c,m), togo-1);

        // ret=true means we find a PhX solution within `togo` steps;
        // early exit is fine since there won't be a shorter PhX solution
        // in iterative deepening search
        if(ret) return true;
    }
    return false;
}

Coord TwoPhaseSolver::ph2_origin_(Coord c) const
{
    // CubieCube transform: Coord::CubieCube2Coord(Coord::Coord2CubieCube(c) * ms);
    // （ Table transform optimization is working for corner only ）
    int corner = c.corner, edge4, edge8;
    // auto cp = Coord::corner2cp(c.corner);
    auto ep = Coord::see2ep(c.slice,c.edge4,c.edge8);
    for(auto &m : get_ph_solution_<Ph1>())
    {
        corner = (*TM.pTMCorner)[m][corner];
        ep = ep * ElementaryMove[m].ep;
    }
    edge4 = Coord::ep2edge4(ep);
    edge8 = Coord::ep2edge8(ep);
    return Coord { 0,0,0,corner,edge4,edge8 };
}

auto TwoPhaseSolver::solve(const Coord &c, int step, bool best)
    -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>
{
    const size_t maxL = std::clamp(step,0,DS);      // largest length allowed
    size_t solL = maxL + 1;                         // smallest length found
    std::array<std::vector<TurnMove>,2> solution;   // solution

    // reset sofar buffer:
    // only once is enough since `set_ph_rsolution(d)` knows exact solution length d
    reset_ph_sofar_<Ph1>();
    reset_ph_sofar_<Ph2>();

    ///
    /// iterative deepening search

    for(auto d1 = distance<Ph1>(c); d1 <= maxL; d1++)
    {
        // start Ph1 search
        bool ret1 = search_phase<Ph1>(c,d1);
        if(!ret1) continue;

        // Ph1 solution found
        set_ph_solution_<Ph1>(d1);

        // start Ph2 search
        auto c2 = ph2_origin_(c);
        size_t togo = (solL > rsolution_[Ph1].first) ? solL - rsolution_[Ph1].first : 0;
        for(auto d2 = distance<Ph2>(c2); d2 < togo; d2++)
        {
            bool ret2 = search_phase<Ph2>(c2,d2);
            if(!ret2) continue;

            // Ph2 solution found
            set_ph_solution_<Ph2>(d2);

            // save solution
            solution[Ph1] = get_ph_solution_<Ph1>();
            solution[Ph2] = get_ph_solution_<Ph2>();
            solL = solution[1].size() + solution[0].size();

            if(!best) goto found;
            if(d2==0) goto found; else break;

            /*
            [Note] To find the twophase-optimal, instead of early exit when a
            solution is found, logically we should continue from the next sibling
            of Ph2 root, instead of the first node of layer under Ph1 root.
            However, we do not implement like this since it makes the search
            algorithm complicated and inefficient.
            */
        }
    }

    if(solL > maxL)
        return {false, {}, {}};

    // solution found
    found:
    return std::make_tuple(true, solution[0], solution[1]);
}

} // namespace cube::solver

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

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CUBE_NO_DEPRECATED
#    define CUBE_NO_DEPRECATED
#  endif
#endif

#endif /* CUBE_EXPORT_H */

#define CUBE_VERSION_FULL  v0.4.0-alpha
#define CUBE_VERSION_MAJOR 0
#define CUBE_VERSION_MINOR 4
#define CUBE_VERSION_PATCH 0
#define CUBE_VERSION_BUILD alpha

#if defined(__GNUC__) || defined(__clang__)
    #define CUBE_EXPORT_FORCE CUBE_EXPORT __attribute__((used))
#else
    #define CUBE_EXPORT_FORCE CUBE_EXPORT
#endif

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

#ifdef __cplusplus
extern "C" {
#endif

CUBE_EXPORT_FORCE inline const char *solve_result_to_string(SolveResult sr) {
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

/*!
 * @brief solve the Rubik's cube

 * @param buf       the buffer to solution (space-seperated moves)
 * @param src       source color configuration, `NULL` means `id`
 * @param tgt       target color configuration, `NULL` means `id`
 * @param step      the max steps to search (30 is recommended;)
 * @param best      try its best to find the short (but slower) solution
 * @return
 *  see enum ``SolveResult``, ``solve_result_to_string``.
 */
CUBE_EXPORT SolveResult solve(
    char*       buf,
    const char* src         Default(CUBE_ID),
    const char* tgt         Default(CUBE_ID),
    int         step        Default(30),
    bool        best        Default(true)
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

namespace cube::show {
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

std::string to_string(const ColorState& cs)
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

std::string to_string(const FaceCube& fc, bool use_digit=true)
{
    if(use_digit){
        return seq2str(fc.f);
    } else {
        return seq2str_fmt(fc.f, [](auto v) { return Face2Str[v]; });
    }
}

std::string to_string(const CubieCube& cc, CubeFormat fmt)
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

std::string to_string(const std::vector<TurnMove>& ms)
{
    return seq2str_fmt(ms, [](auto v){
        return Move2Str[v];
    }, " ");
}

} // namespace cube::show

using namespace cube;

const std::string_view cid = CUBE_ID;

cube::solver::TwoPhaseSolver TPS;

std::string apply_moves(std::string_view s, const std::vector<TurnMove> &ms)
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

SolveResult solve(
    char* buf, const char *src, const char* tgt, int step, bool best)
{
    auto s_src = src == NULL ? cid : std::string_view(src);
    auto s_tgt = tgt == NULL ? cid : std::string_view(tgt);

    // invalid cube
    if(s_src != cid && !cube::utils::is_valid_config(s_src)) return SolveResultInvalidSrc;
    if(s_tgt != cid && !cube::utils::is_valid_config(s_tgt)) return SolveResultInvalidTgt;

    // trivial cube
    if(s_src == s_tgt) { buf[0] = '\0'; return SolveResultSuccess; }

    auto cc_src = ColorState::fromString(s_src).toCubieCube();
    auto cc_tgt = ColorState::fromString(s_tgt).toCubieCube();
    CubieCube cc = ~cc_tgt*cc_src;

    // unsolvable cube
    if(!cc.isSolvable()) return SolveResultUnsolvable;

    const auto & [found, s1, s2] = TPS.solve(cube::pdb::Coord::CubieCube2Coord(cc), step, best);

    // solution is not found since the search depth is too small
    if(!found) return SolveResultNotFound;

    std::vector<TurnMove> sol = [](const auto &s1, const auto &s2) {
        std::vector<TurnMove> solution;
        size_t n1 = s1.size(), n2 = s2.size();
        // if the transition moves of ph1-ph2 are homogeneous, combine them
        if(!s1.empty() && !s2.empty() && s1[n1-1]/3 == s2[0]/3) {
            std::copy(s1.begin(), s1.end()-1, std::back_inserter(solution));
            int m = (s1[n1-1] + s2[0]- s2[0]/3 *6 +2) %4;
            if(m!=0) solution.push_back(static_cast<TurnMove>(s2[0]/3*3+m-1));
            std::copy(s2.begin()+1, s2.end(), std::back_inserter(solution));
        } else {
            std::copy(s1.begin(), s1.end(), std::back_inserter(solution));
            std::copy(s2.begin(), s2.end(), std::back_inserter(solution));
        }
        return solution;
    }(s1,s2);

    auto s = show::to_string(sol);
    std::copy(s.cbegin(), s.cend(), buf);
    buf[s.length()] = '\0';
    return SolveResultSuccess;
}

bool solvable(const char* cube)
{
    return cube::utils::is_valid_config<std::string_view>(cube)
           && ColorState::fromString(cube).toCubieCube().isSolvable();
}

bool facecube(char* buf, const char *maneuver, const char *cube)
{
    if(!buf) return false;

    std::string cube_str = cube ? std::string(cube) : std::string(CUBE_ID);
    std::string maneuver_str = maneuver ? std::string(maneuver) : std::string("");

    if(cube_str.size()!= 54 || !cube::utils::is_valid_maneuver(maneuver_str)) {
        buf[0] = '\0';
        return false;
    }

    const auto ms = cube::utils::parse_manuever(maneuver_str);
    const auto color = apply_moves(cube_str, ms);

    std::copy(color.cbegin(), color.cend(), buf);
    buf[color.length()] = '\0';
    return true;
}

bool permutation(char* buf, const char* ms_or_cube, int format)
{
    if(!buf) return false;

    std::string cube_str{};
    if(ms_or_cube) {
        if(cube::utils::is_valid_config<std::string_view>(ms_or_cube)) {
            cube_str = std::string(ms_or_cube);
        } else if(cube::utils::is_valid_maneuver(ms_or_cube)) {
            auto ms = cube::utils::parse_manuever(ms_or_cube);
            cube_str = apply_moves(cid,ms);
        } else {
            buf[0] = '\0';
            return false;
        }
    } else {
        cube_str = cid;
    }

    std::string perm_str{};
    auto cc = ColorState::fromString(cube_str).toCubieCube();
    perm_str = show::to_string(cc, show::CubeFormat(format));

    std::copy(perm_str.cbegin(), perm_str.cend(), buf);
    buf[perm_str.length()] = '\0';
    return true;
}
namespace cube::internal {

namespace fs = std::filesystem;

static std::string& get_custom_dir_storage()
{
    static std::string custom_dir = "";
    return custom_dir;
}

void set_table_dir(std::string_view dir)
{
    get_custom_dir_storage() = dir;
}

fs::path get_table_dir()
{
    // user-settings
    std::string user_dir = get_custom_dir_storage();
    if(!user_dir.empty()) return fs::path(user_dir);

    // environment variable
    if(const char *env = std::getenv("CUBE_TABLE_DIR")) {
        if(env[0] != '\0') return fs::path(env);
    }

    // system cache dir
    try {
        return cube::utils::get_cache_dir() / "cube" / "tables";
    } catch (...) {}

    // fallback: current dir
    return fs::current_path() / "tables";
}

bool is_table_ready()
{
    return fs::exists(get_table_dir() / "tm_twist.dat");
}

void preload_tables()
{
    (void)pdb::get_TM();
    (void)pdb::get_TP();
}

} // namespace cube::internal