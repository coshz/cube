#pragma once
#include <vector>
#include <array>
#include <set>
#include <algorithm>
#include <utility>
#include <numeric>
#include <stdexcept>

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