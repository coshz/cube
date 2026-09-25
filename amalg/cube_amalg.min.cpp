/*
 * File: cube_amalg.min.cpp
 * Project: cube
 * Author: coshz <fsinhx@gmail.com>
 * Version: v0.4.2
 * Date: 2026-09-26
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
#include <cstdint>
#include <string_view>
#include <set>
#include <filesystem>
#include <type_traits>
#include <limits>
#include <fstream>
#include <ios>
#include <tuple>
#include <cstdlib>
#include <chrono>
#include <optional>
#include <string>
#include <cstring>
#include <regex>
#include <sstream>
#include <stdbool.h>
#include <stdint.h>

namespace cube::math {
using std::size_t;
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
constexpr auto takeByRange(const std::array<T,N>& xs) -> std::array<T,End-Begin+1>
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
}
namespace cube::data {
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
const CornerCubie CornerCubieMove[6] = {
    {{UBR,0},{URF,0},{UFL,0},{ULB,0},{DFR,0},{DLF,0},{DBL,0},{DRB,0}},
    {{DFR,2},{UFL,0},{ULB,0},{URF,1},{DRB,1},{DLF,0},{DBL,0},{UBR,2}},
    {{UFL,1},{DLF,2},{ULB,0},{UBR,0},{URF,2},{DFR,1},{DBL,0},{DRB,0}},
    {{URF,0},{UFL,0},{ULB,0},{UBR,0},{DLF,0},{DBL,0},{DRB,0},{DFR,0}},
    {{URF,0},{ULB,1},{DBL,2},{UBR,0},{DFR,0},{UFL,2},{DLF,1},{DRB,0}},
    {{URF,0},{UFL,0},{UBR,1},{DRB,2},{DFR,0},{DLF,0},{ULB,2},{DBL,1}}
};
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
    GN_HTM      = 20,
    GN_QTM      = 26,
    N_MOVE      = 18,
    N_TWIST     = 2187,
    N_FLIP      = 2048,
    N_SLICE     = 495,
    N_CORNER    = 40320,
    N_EDGE8     = 40320,
    N_EDGE4     = 24,
    N_SYM       = 48,
    N_SYM_D4h   = 16,
    EQ_FLIPSLICE= 64430,
    EQ_CORNER   = 2768,
};
using std::size_t;
using namespace cube::math;
template<size_t N,typename T=int>
struct Perm
{
    using value_type = T;
    static constexpr size_t size() { return N; }
    constexpr Perm() noexcept
    {
        for(size_t i = 0; i < N; ++i) X[i] = static_cast<T>(i);
    }
    constexpr Perm(const std::array<T,N>& xs) noexcept
    :X{xs}
    {
        assert(isValidPermutation<N>(xs) && "array must be a valid permutation");
    }
    constexpr Perm(std::initializer_list<T> xs) noexcept
    {
        assert(xs.size() == N && "size mismatch");
        auto dit = X.begin();
        for(auto it = xs.begin(); it != xs.end(); it++, dit++) { *dit = *it; }
        assert(isValidPermutation<N>(X) && "invalid permutation list");
    }
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
    constexpr Perm operator~() const
    {
        return { forepermute(Perm::id.X, this->X) };
    }
    constexpr T& operator[](size_t idx)             { return X[idx]; }
    constexpr const T& operator[](size_t idx) const { return X[idx]; }
    bool parity() const
    {
        const auto [fixed, cs] = decomposite(this->X);
        int s = 1;
        for(auto &c:cs) s *= (c.size() % 2 == 0) ? -1 : 1;
        return s == 1;
    }
    size_t order() const { return orderOf(X); }
    constexpr size_t rank() const { return rankOf(X); }
    static constexpr Perm<N,T> fromRank(size_t i)
    { return { cube::math::fromRank<T,N>(i) }; }
    static const Perm id;
    std::array<T,N> X{};
};
template<size_t N,typename T>
inline constexpr Perm<N,T> Perm<N,T>::id{};
template<size_t N, size_t L, typename T=int>
struct CArray
{
    using value_type = T;
    static constexpr size_t period = N;
    static constexpr size_t size() noexcept { return L; }
    constexpr CArray() noexcept {}
    constexpr CArray(const std::array<T,L> &arr) noexcept
    :xs{arr}
    {
        auto n = static_cast<T>(N);
        for(size_t i = 0; i < L; i++) { xs[i] = (xs[i] % n + n) % n; }
    }
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
    friend constexpr CArray operator+(CArray lhs, const CArray& rhs)
    {
        return lhs += rhs;
    }
    constexpr CArray operator~() const noexcept
    {
        CArray<N,L,T> inv {};
        auto n = static_cast<T>(N);
        for(size_t i = 0; i < L; i++) inv[i] = (n - xs[i]) % n;
        return inv;
    }
    constexpr T sum() const noexcept
    {
        T s = 0;
        for(size_t i = 0; i < L; i++) s += xs[i];
        return s % N;
    }
    static const CArray id;
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
}
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
    Layer & operator[](size_t i) noexcept           { return s[i]; }
    const Layer operator[](size_t i) const noexcept { return s[i]; }
    static ColorState fromString(std::string_view cube);
    static bool is_valid_config(std::string_view cube);
    FaceCube  toFaceCube() const;
    CubieCube toCubieCube() const;
    static const ColorState id;
};
inline const ColorState ColorState::id = {{U,U,U,U,U,U,U,U,U,R,R,R,R,R,R,R,R,R,F,F,F,F,F,F,F,F,F,D,D,D,D,D,D,D,D,D,L,L,L,L,L,L,L,L,L,B,B,B,B,B,B,B,B,B}};
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
inline constexpr CornerPerm    eCP = {0,1,2,3,4,5,6,7};
inline constexpr EdgePerm      eEP = {0,1,2,3,4,5,6,7,8,9,10,11};
inline constexpr CornerOri     eCO = {0,0,0,0,0,0,0,0};
inline constexpr EdgeOri       eEO = {0,0,0,0,0,0,0,0,0,0,0,0};
inline constexpr FacePerm      eFP = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53};
inline const FaceCube   FaceCube::id = FaceCube{eFP};
inline const CubieCube  CubieCube::id = CubieCube{eCP,eCO,eEP,eEO};
inline constexpr CubieCube mU = {{3,0,1,2,4,5,6,7},{0,0,0,0,0,0,0,0},{3,0,1,2,4,5,6,7,8,9,10,11},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr CubieCube mR = {{4,1,2,0,7,5,6,3},{2,0,0,1,1,0,0,2},{8,1,2,3,11,5,6,7,4,9,10,0},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr CubieCube mF = {{1,5,2,3,0,4,6,7},{1,2,0,0,2,1,0,0},{0,9,2,3,4,8,6,7,1,5,10,11},{0,1,0,0,0,1,0,0,1,1,0,0}};
inline constexpr CubieCube mD = {{0,1,2,3,5,6,7,4},{0,0,0,0,0,0,0,0},{0,1,2,3,5,6,7,4,8,9,10,11},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr CubieCube mL = {{0,2,6,3,4,1,5,7},{0,1,2,0,0,2,1,0},{0,1,10,3,4,5,9,7,8,2,6,11},{0,0,0,0,0,0,0,0,0,0,0,0}};
inline constexpr CubieCube mB = {{0,1,3,7,4,5,2,6},{0,0,1,2,0,0,2,1},{0,1,2,11,4,5,6,10,8,9,3,7},{0,0,0,1,0,0,0,1,0,0,1,1}};
inline constexpr std::array<CubieCube,18>
    ElementaryMove = { mU,mU*mU,mU*mU*mU,mR,mR*mR,mR*mR*mR,mF,mF*mF,mF*mF*mF,mD,mD*mD,mD*mD*mD,mL,mL*mL,mL*mL*mL,mB,mB*mB,mB*mB*mB };
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
    ColorState cs = c;
    for (auto m : ms) cs = ElementaryPerm[m].f.act_right(cs);
    return cs;
}
}
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
    if(cfg.size() != 54) return false;
    std::set<char> vs { cfg[CC[0]],cfg[CC[1]],cfg[CC[2]],cfg[CC[3]],cfg[CC[4]],cfg[CC[5]] };
    if(vs.size() != 6) return false;
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
}
namespace cube::pdb {
struct Coord
{
    int twist, flip, slice;
    int corner, edge4, edge8;
    static const Coord id;
    static CornerOri    twist2co(int);
    static int          co2twist(const CornerOri &);
    static EdgeOri      flip2eo(int);
    static int          eo2flip(const EdgeOri &);
    static CornerPerm   corner2cp(int);
    static int          cp2corner(const CornerPerm &);
    static int          ep2slice(const EdgePerm &);
    static int          ep2edge4(const EdgePerm &);
    static int          ep2edge8(const EdgePerm &);
    static EdgePerm     slice2ep(int);
    static EdgePerm     edge42ep(int);
    static EdgePerm     edge82ep(int);
    static EdgePerm     see2ep(int,int,int);
    static Coord        CubieCube2Coord(const CubieCube &);
    static CubieCube    Coord2CubieCube(const Coord &);
};
constexpr bool operator==(const Coord &c1, const Coord &c2)
{
    return c1.twist == c2.twist && c1.flip == c2.flip && c1.slice == c2.slice
        && c1.corner == c2.corner && c1.edge4 == c2.edge4 && c1.edge8 == c2.edge8;
}
inline constexpr Coord Coord::id = { 0,0,0,0,0,0 };
}
namespace cube::pdb {
using namespace cube::math;
using namespace cube::data;
inline bool isSliceEdge(size_t idx)
{
    return FR <= idx && idx <= BR;
}
int Coord::co2twist(const CornerOri &co)
{
    return static_cast<int>(fromDigits<3>(takeByRange<1,7>(co.xs)));
}
CornerOri Coord::twist2co(int i)
{
    CornerOri co;
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
    auto sub1to11 = CArray<2,11,CornerOri::value_type>{toDigits<2,11,CornerOri::value_type>(i)};
    std::copy(sub1to11.xs.begin(),sub1to11.xs.end(),eo.xs.begin()+1);
    eo[0] = (2 - sub1to11.sum()) % 2;
    return eo;
}
int Coord::ep2slice(const EdgePerm &ep)
{
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
    for(size_t i = 0, j = 0; i < 12; i++){
        ep[i] = (i == si[j]) ? 8 + j++ : (EdgePerm::value_type) ~0UL;
    }
    return ep;
}
EdgePerm Coord::edge42ep(int i)
{
    EdgePerm ep;
    auto e4 = Perm<4,EdgePerm::value_type>::fromRank(i);
    for(size_t i = 0; i < 12; i++){
        ep[i] = (i < 8) ? (EdgePerm::value_type) ~0UL : e4[i-8] + 8;
    }
    return ep;
}
EdgePerm Coord::edge82ep(int i)
{
    EdgePerm ep;
    auto e8 = Perm<8,EdgePerm::value_type>::fromRank(i);
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
}
namespace cube::config {
void set_table_dir(std::filesystem::path dir);
auto get_table_dir() -> std::filesystem::path;
}
namespace cube::pdb {
using namespace cube::data;
namespace fs = std::filesystem;
typedef uint16_t    mt_value_t;
typedef uint8_t     pt_value_t;
template <typename Table> void save_to(const Table &table, fs::path path);
template <typename Table> void load_from(Table &table, fs::path path);
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
template<typename T=mt_value_t>
struct TableMove
{
    static_assert(std::is_integral<T>::value);
    static_assert(std::numeric_limits<T>::digits >= 16);
    using value_t = T;
    TableMove(fs::path dir = {});
    TableMove(const TableMove &) = delete;
    ~TableMove();
    TableMove& operator=(const TableMove &) = delete;
    template<typename Table, typename F1, typename F2>
    std::enable_if_t<Table::shape[0] == N_MOVE, void>
    buildMoveTable(Table &t, F1&& coord2i, F2&& i2coord, fs::path filename);
    static bool is_ready(fs::path);
    const fs::path tdir;
    NArray<T,N_MOVE,N_TWIST>   *pTMTwist;
    NArray<T,N_MOVE,N_FLIP>    *pTMFlip;
    NArray<T,N_MOVE,N_SLICE>   *pTMSlice;
    NArray<T,N_MOVE,N_CORNER>  *pTMCorner;
    NArray<T,N_MOVE,N_EDGE4>   *pTMEdge4;
    NArray<T,N_MOVE,N_EDGE8>   *pTMEdge8;
};
template<typename T=pt_value_t>
struct TablePrunning
{
    using value_type = T;
    TablePrunning(fs::path dir = {});
    TablePrunning(const TablePrunning &) = delete;
    ~TablePrunning();
    TablePrunning operator=(const TablePrunning &) = delete;
    template<typename Table, typename MT1, typename MT2>
    std::enable_if_t<Table::shape[0] == MT1::shape[1] && Table::shape[1] == MT2::shape[1]>
    buildPrunningTable(Table &t, const MT1 &mt1, const MT2 &mt2, fs::path filename);
    static bool is_ready(fs::path);
    const fs::path tdir;
    NArray<T,N_SLICE,N_FLIP>   *pTPSliceFlip;
    NArray<T,N_SLICE,N_TWIST>  *pTPSliceTwist;
    NArray<T,N_EDGE4,N_EDGE8>  *pTPEdge4Edge8;
    NArray<T,N_EDGE4,N_CORNER> *pTPEdge4Corner;
};
template<typename T=mt_value_t>
using SingletonTM = Singleton<TableMove<T>>;
template<typename T=pt_value_t>
using SingletonTP = Singleton<TablePrunning<T>>;
inline const auto &get_TM() { return SingletonTM<>::instance(); }
inline const auto &get_TP() { return SingletonTP<>::instance(); }
inline bool tables_ready(fs::path dir = {})
{
    return TableMove<>::is_ready(dir) && TablePrunning<>::is_ready(dir);
}
inline void preload_tables()
{
    (void) get_TM();
    (void) get_TP();
}
}
#if defined(VERBOSE) && VERBOSE
#define VPRINT(...) printf(__VA_ARGS__)
#else
#define VPRINT(...)
#endif
namespace cube::pdb {
using namespace cube::data;
auto table_dir_or_default(fs::path dir) -> fs::path
{
    return dir.empty() ? cube::config::get_table_dir() : dir;
}
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
TableMove<T>::buildMoveTable(Table &t, F1&& coord2i, F2&& i2coord, fs::path filename)
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
TableMove<T>::TableMove(fs::path dir)
:tdir{table_dir_or_default(dir)}
{
    VPRINT("INIT MOVE TABLES -- \n");
    pTMTwist     = new NArray<T,N_MOVE,N_TWIST>;
    pTMFlip      = new NArray<T,N_MOVE,N_FLIP>;
    pTMSlice     = new NArray<T,N_MOVE,N_SLICE>;
    pTMCorner    = new NArray<T,N_MOVE,N_CORNER>;
    pTMEdge4     = new NArray<T,N_MOVE,N_EDGE4>;
    pTMEdge8     = new NArray<T,N_MOVE,N_EDGE8>;
    if(!is_ready(tdir)) {
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
bool TableMove<T>::is_ready(fs::path tdir)
{
    tdir = table_dir_or_default(tdir);
    return fs::exists(tdir/"tm_twist.dat") &&
           fs::exists(tdir/"tm_flip.dat") &&
           fs::exists(tdir/"tm_slice.dat") &&
           fs::exists(tdir/"tm_corner.dat") &&
           fs::exists(tdir/"tm_edge4.dat") &&
           fs::exists(tdir/"tm_edge8.dat");
}
template<typename T>
template<typename Table, typename MT1, typename MT2>
std::enable_if_t<Table::shape[0] == MT1::shape[1] && Table::shape[1] == MT2::shape[1]>
TablePrunning<T>::buildPrunningTable(
    Table &t, const MT1 &mt1, const MT2 &mt2, fs::path filename)
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
        for(size_t i = 0; i < t.shape[0]; i++) for(size_t j = 0; j < t.shape[1]; j++)
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
TablePrunning<T>::TablePrunning(fs::path dir)
:tdir{table_dir_or_default(dir)}
{
    VPRINT("INIT PRUNNING TABLES -- \n");
    pTPSliceFlip     = new NArray<T,N_SLICE,N_FLIP>;
    pTPSliceTwist    = new NArray<T,N_SLICE,N_TWIST>;
    pTPEdge4Edge8    = new NArray<T,N_EDGE4,N_EDGE8>;
    pTPEdge4Corner   = new NArray<T,N_EDGE4,N_CORNER>;
    if(!is_ready(tdir)) {
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
template<typename T>
bool TablePrunning<T>::is_ready(fs::path tdir)
{
    tdir = table_dir_or_default(tdir);
    return fs::exists(tdir/"tp_slicetwist.dat") &&
           fs::exists(tdir/"tp_sliceflip.dat") &&
           fs::exists(tdir/"tp_edge4corner.dat") &&
           fs::exists(tdir/"tp_edge4edge8.dat");
}
template struct TableMove<>;
template struct TablePrunning<>;
}
namespace cube::solver {
using namespace cube::data;
using namespace cube::pdb;
class TwoPhaseSolver
{
public:
    auto solve(const Coord &c, int step, bool best)
        -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>;
protected:
    enum enum_phase { Ph1=0, Ph2=1 };
    template<enum_phase PhX> bool search_phase(const Coord &c, size_t togo);
    template<enum_phase PhX> static Coord transform(const Coord &c, const TurnMove &m);
    template<enum_phase PhX> static size_t distance(const Coord &c);
    static constexpr int D0 = 12, D1 = 18, DS = D0+D1;
    template<enum_phase PhX> static constexpr auto& D = std::get<PhX>(std::tie(D0,D1));
    static constexpr std::array<TurnMove,18> EM0
        = { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
    static constexpr std::array<TurnMove,10> EM1
        = { Ux1,Ux2,Ux3,Rx2,Fx2,Dx1,Dx2,Dx3,Lx2,Bx2 };
    template<enum_phase PhX>
    static constexpr auto& EM = std::get<PhX>(std::tie(EM0,EM1));
private:
    template<enum_phase PhX>
    void reset_ph_sofar_()  { sofar_[PhX].fill(-1); }
    template<enum_phase PhX>
    void set_ph_solution_(size_t L)
    {
        rsolution_[PhX].first = L;
        std::copy(sofar_[PhX].begin(), sofar_[PhX].begin()+L, rsolution_[PhX].second.begin());
    }
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
    Coord ph2_origin_(Coord c) const;
    std::array<std::array<int,DS+2>,2>                  sofar_;
    std::array<std::pair<size_t,std::array<int,DS>>,2>  rsolution_;
};
}
namespace cube::solver {
#define TM cube::pdb::get_TM()
#define TP cube::pdb::get_TP()
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
        -1,-1,-1
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
        if(is_dull_triple(m,sofar_[PhX][togo],sofar_[PhX][togo+1])) continue;
        sofar_[PhX][togo-1] = m;
        bool ret = search_phase<PhX>(transform<PhX>(c,m), togo-1);
        if(ret) return true;
    }
    return false;
}
Coord TwoPhaseSolver::ph2_origin_(Coord c) const
{
    int corner = c.corner, edge4, edge8;
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
    const size_t maxL = std::clamp(step,0,DS);
    size_t solL = maxL + 1;
    std::array<std::vector<TurnMove>,2> solution;
    reset_ph_sofar_<Ph1>();
    reset_ph_sofar_<Ph2>();
    for(auto d1 = distance<Ph1>(c); d1 <= maxL; d1++)
    {
        bool ret1 = search_phase<Ph1>(c,d1);
        if(!ret1) continue;
        set_ph_solution_<Ph1>(d1);
        auto c2 = ph2_origin_(c);
        size_t togo = (solL > rsolution_[Ph1].first) ? solL - rsolution_[Ph1].first : 0;
        for(auto d2 = distance<Ph2>(c2); d2 < togo; d2++)
        {
            bool ret2 = search_phase<Ph2>(c2,d2);
            if(!ret2) continue;
            set_ph_solution_<Ph2>(d2);
            solution[Ph1] = get_ph_solution_<Ph1>();
            solution[Ph2] = get_ph_solution_<Ph2>();
            solL = solution[1].size() + solution[0].size();
            if(!best) goto found;
            if(d2==0) goto found; else break;
        }
    }
    if(solL > maxL)
        return {false, {}, {}};
    found:
    return std::make_tuple(true, solution[0], solution[1]);
}
}
namespace cube::utils {
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
}
namespace cube::config
{
namespace fs = std::filesystem;
static fs::path& get_custom_dir_storage()
{
    static fs::path custom_dir = "";
    return custom_dir;
}
void set_table_dir(fs::path dir)
{
    get_custom_dir_storage() = dir;
}
fs::path get_table_dir()
{
    const auto user_dir = get_custom_dir_storage();
    if(!user_dir.empty()) return user_dir;
    if(const char *env = std::getenv("CUBE_TABLE_DIR")) {
        if(env[0] != '\0') return fs::path(env);
    }
    try {
        return utils::get_cache_dir() / "cube.coshz" / "tables";
    } catch (...) {
        return fs::absolute("tables");
    }
}
}
namespace cube::internal
{
using namespace cube::data;
std::string apply_maneuver(std::string_view s, const std::vector<TurnMove>& ms);
auto parse_maneuver(std::string_view s) -> std::vector<TurnMove>;
inline auto operator""_Tm(const char* ts, size_t n) -> std::vector<TurnMove>
{
    return parse_maneuver(std::string(ts,n));
}
bool is_valid_maneuver(std::string_view);
}
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
}
}
#ifndef CUBE_EXPORT_H
#define CUBE_EXPORT_H
#ifdef CUBE_STATIC_DEFINE
#  define CUBE_EXPORT
#  define CUBE_NO_EXPORT
#else
#  ifndef CUBE_EXPORT
#    ifdef cube_EXPORTS
#      define CUBE_EXPORT __attribute__((visibility("default")))
#    else
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
#if 0
#  ifndef CUBE_NO_DEPRECATED
#    define CUBE_NO_DEPRECATED
#  endif
#endif
#endif
#ifndef __cplusplus
#error "This is a C++ header; please use a C++ compiler OR include cube/cube.h instead"
#endif
namespace cube {
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
    bool is_success() const noexcept {
        return status == SolveResult::Success;
    }
};
CUBE_EXPORT bool is_solvable(std::string_view color_cube);
[[nodiscard]] CUBE_EXPORT Solution solve(
    std::string_view src,
    std::string_view tgt = cubeId,
    int step = 30,
    bool best = true
);
[[nodiscard]] CUBE_EXPORT std::string apply_maneuver(
    std::string_view maneuver,
    std::string_view cube = cubeId
);
[[nodiscard]] CUBE_EXPORT std::string show_permutation(
    std::string_view ms_or_cube,
    PermFormat format = PermFormat::Cycle
);
}
namespace cube {
CUBE_EXPORT void set_table_dir(std::filesystem::path dir);
CUBE_EXPORT auto get_table_dir() -> std::filesystem::path;
CUBE_EXPORT bool tables_ready();
CUBE_EXPORT void preload_tables();
CUBE_EXPORT bool is_valid_config(std::string_view cube);
CUBE_EXPORT bool is_valid_maneuver(std::string_view maneuver);
}
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
    default:
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
}
namespace cube {
using namespace cube::data;
namespace fs = std::filesystem;
void set_table_dir(fs::path dir) { config::set_table_dir(dir); }
auto get_table_dir() -> fs::path { return config::get_table_dir(); }
bool tables_ready() { return pdb::tables_ready(); }
void preload_tables() { pdb::preload_tables(); }
bool is_valid_config(std::string_view cube) { return ColorState::is_valid_config(cube); }
bool is_valid_maneuver(std::string_view maneuver) { return internal::is_valid_maneuver(maneuver); }
cube::solver::TwoPhaseSolver TPS;
Solution solve(std::string_view src, std::string_view tgt, int step, bool best) {
    if (src != cubeId && !is_valid_config(src)) return {SolveResult::InvalidSrc, ""};
    if (tgt != cubeId && !is_valid_config(tgt)) return {SolveResult::InvalidTgt, ""};
    if (src == tgt) return {SolveResult::Success, ""};
    auto cc_src = ColorState::fromString(src).toCubieCube();
    auto cc_tgt = ColorState::fromString(tgt).toCubieCube();
    CubieCube cc = ~cc_tgt * cc_src;
    if (!cc.isSolvable()) return {SolveResult::Unsolvable, ""};
    const auto& [found, s1, s2] = TPS.solve(pdb::Coord::CubieCube2Coord(cc), step, best);
    if (!found) return {SolveResult::NotFound, ""};
    std::vector<TurnMove> sol;
    size_t n1 = s1.size(), n2 = s2.size();
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
    return show::to_string(cc, show::CubeFormat(static_cast<int>(format)));
}
}
#define CUBE_VERSION_FULL  v0.4.2
#define CUBE_VERSION_MAJOR 0
#define CUBE_VERSION_MINOR 4
#define CUBE_VERSION_PATCH 2
#define CUBE_VERSION_BUILD
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
#define CUBE_BS 128
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
CUBE_EXPORT const char *solve_result_to_string(int32_t sr);
CUBE_EXPORT SolveResult solve(
    char*       buf,
    const char* src         Default(CUBE_ID),
    const char* tgt         Default(CUBE_ID),
    int         step        Default(30),
    bool        best        Default(true)
);
CUBE_EXPORT bool solvable(
    const char* color_cube  Default(CUBE_ID)
);
CUBE_EXPORT bool facecube(
    char* buf,
    const char* maneuver    Default(""),
    const char *cube        Default(CUBE_ID)
);
CUBE_EXPORT bool permutation(
    char*       buf,
    const char* ms_or_cube  Default(CUBE_ID),
    int         format      Default(2)
);
#ifdef __cplusplus
}
#endif
extern "C" {
const char* solve_result_to_string(int sr)
{
    return solve_result_info(static_cast<SolveResult>(sr));
}
SolveResult solve(
    char* buf, const char *src, const char* tgt, int step, bool best)
{
    if(!buf) return SolveResultUnknownErr;
    auto s_src = src == NULL ? cube::cubeId : std::string_view(src);
    auto s_tgt = tgt == NULL ? cube::cubeId : std::string_view(tgt);
    auto res = cube::solve(s_src, s_tgt, step, best);
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
}