#pragma once

#include "help.hpp"

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
