#pragma once
#include "common.hpp"
#include <array>
#include <initializer_list>
#include <cassert>
#include <cstddef>
#include <type_traits>

namespace cube::data {
#include "data.inc"

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
