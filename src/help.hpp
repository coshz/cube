#pragma once
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <set>
#include <cassert>
#include <algorithm>

///
///////////////////////////////  Declarations  ///////////////////////////////
///

/* convert sequence to string */
template<typename VectorLike> 
std::string seq2str(const VectorLike &xs,
    std::string pre="", std::string suf="", std::string sep="");

/* the cycle decompositon of permutation */
template<typename VectorLike> 
auto decomposite(const VectorLike &xs) -> std::vector<std::vector<typename VectorLike::value_type>>;

/* the order (period) of permutation */
template<typename T, size_t N> 
size_t orderOf(const std::array<T,N> &xs);

/* the rank of permutation */
template<typename T, size_t N>
size_t rankOf(const std::array<T,N> &xs);

/* create a permutation from rank */
template<typename T, size_t N>
std::array<T,N> fromRank(size_t i);

/* creat a list by applying f for N times */
template<typename F, typename T, size_t N>
std::array<T,N+1> nestList(F &&f, T &&x);

/* take subarray of index range [Begin,End] */
template<size_t Begin, size_t End, typename T, size_t N>
constexpr auto takeByRange(const std::array<T,N>&) -> std::array<T,End-Begin+1>;

template<size_t B, typename Int, size_t N>
constexpr auto fromDigits(const std::array<Int,N> &xs) -> size_t;

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

/* the (passive) permutation on Sn */
template<size_t N,typename T=int>
struct Perm
{
    using value_type = T;

    inline constexpr T& operator[](size_t idx);
    inline constexpr const T& operator[](size_t idx) const;

    /* the parity of permuation: even => true, odd => false */
    bool parity() const;

    /* the order of a permutation */
    size_t order() const;

    /* the rank of a permutation */
    size_t rank() const;

    /* create Perm from rank */
    static Perm<N,T> fromRank(size_t i);

    static constexpr size_t size() { return N; }

    std::array<T,N> X;  
};

/* the array on Cn */
template<size_t N, size_t L, typename T=int>
struct CArray
{
    using valut_type = T;
    static constexpr int period = N;

    inline constexpr T& operator[](size_t idx);
    inline constexpr const T& operator[](size_t idx) const;

    /* the sum of sequences */
    int sum() const;
    
    static constexpr size_t size() { return L; }

    std::array<T,L> xs;
};

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

template<size_t N,typename T>
inline bool operator==(const Perm<N,T> &lhs, const Perm<N,T> &rhs);

template<size_t N, size_t L, typename T>
inline bool operator==(const CArray<N,L,T>&lhs, const CArray<N,L,T>&rhs);

/* the multiplication of permutations */
template<size_t N, typename T>
constexpr Perm<N,T> operator*(const Perm<N,T>&X, const Perm<N,T> &Y);

/* the inverse of multiplication  */
template<size_t N, typename T>
constexpr Perm<N,T> operator~(const Perm<N,T>&X);

/* the perm (right action) on CArray */
template<size_t N, size_t L, typename T>
constexpr CArray<N,L,T> operator*(const CArray<N,L,T>&xs, const Perm<L,T>&X);

/* (module) addition on CArray*/
template<size_t N, size_t L, typename T>
constexpr CArray<N,L,T> operator+(const CArray<N,L,T>&xs, const CArray<N,L,T>&ys);

/* the inverse of addition */
template<size_t N, size_t L, typename T>
constexpr CArray<N,L,T> operator-(const CArray<N,L,T>&xs);

///
/////////////////////////////// Implimentations ///////////////////////////////
///

template<typename VectorLike> 
std::string seq2str(const VectorLike &xs, 
    std::string pre, std::string suf, std::string sep)
{
    if(xs.size() == 0) return std::string("");
    std::stringstream ss;
    ss << pre << xs[0];
    for(size_t i = 1; i < xs.size(); i++) ss << sep << xs[i];
    ss << suf;
    return ss.str();
}

template<typename VectorLike>
auto decomposite(const VectorLike &xs) -> std::vector<std::vector<typename VectorLike::value_type>>
{
    using T = typename VectorLike::value_type;
    if(xs.size() == 0) return std::vector<std::vector<T>>(0);
    
    std::vector<size_t> fs(xs.size(),0);
    size_t i, j, f = 0;
    for(i = 0; i < xs.size(); i++)
    {
        if(fs[i] != 0 || i == (size_t) xs[i]) continue;
        fs[i] = ++f;
        for(j=xs[i]; j!=i; j=xs[j]) fs[j] = f;
    }

    std::vector<std::vector<T>> cycles(f+1);

    for(size_t k=0; k<xs.size(); k++)
    {
        cycles[fs[k]].push_back((T)k);
    }
    // keep fixed-points at positon 0
    std::sort(cycles.begin() + 1, cycles.end(), [](auto &v1, auto &v2){
        return v1.size() > v2.size();
    });
    return cycles;
}

template<typename T, size_t N>
size_t orderOf(const std::array<T,N> &xs)
{
    if(xs.size() == 0) return 1;
    
    auto lcm = [](size_t x, size_t y){
        if(x < y) { x^=y, y^=x, x ^= y; }
        size_t p = x, q = y, r;
        do {
            r = p % q;
            p = q, q = r;
        }
        while(q != 0);
        return x / p * y;
    };

    size_t m = 1;
    auto cs = decomposite(xs);
    for(size_t i = 1; i < cs.size(); i++) { m = lcm(m, cs[i].size()); }
    
    return m;
}

template<typename T, size_t N>
size_t rankOf(const std::array<T,N> &xs)
{
    size_t r = 0;
    std::set<T> s(xs.begin(),xs.end());
    for(size_t k = 0; k < N; k++) {
        auto it = s.find(xs[k]);
        assert(it != s.end());
        r += std::distance(s.begin(), it) * factorial(N - k - 1);
        s.erase(it);
    }
    return r;
}

template<typename T, size_t N>
std::array<T,N> fromRank(size_t i)
{
    std::array<T,N> p;
    std::set<T> s;
    for(size_t k = 0; k < N; k++) s.insert((T)k);
    for(size_t k = 0; k < N; k++) {
        auto f = factorial(N-k-1);
        auto it = std::next(s.begin(), i/f);
        p[k] = *it;
        i = i % f;
        s.erase(it);
    }
    return p;
}

template<typename F, typename T, size_t N>
std::array<T,N+1> nestList(F &&f, T &&x)
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
    std::array<size_t,M> X;
    size_t n=rank, k=0;
    for(size_t i = 0; i < M; i++) {
        while(binomial(N-1-k,M-i) > n || n >= binomial(N-k,4-i)) k++;
        X[i] = k;
        n -= binomial(N-1-k,M-i);
        k += 1;
    }
    return X;
}

template<size_t N,typename T>
bool Perm<N,T>::parity() const
{
    auto cs = decomposite(this->X);
    /* sgn = (N - k) % 2 */
    return (cs.size() + cs[0].size() - N) % 2;
}

template<size_t N,typename T>
size_t Perm<N,T>::order() const
{
    return orderOf(this->X);
}

template<size_t N,typename T>
size_t Perm<N,T>::rank() const
{
    return rankOf(this->X);
}

template<size_t N, typename T>
Perm<N,T> Perm<N,T>::fromRank(size_t i)
{
    return Perm<N,T> { ::fromRank<T,N>(i) };
}

template<size_t N,typename T>
constexpr T& Perm<N,T>::operator[](size_t idx)
{
    return this->X[idx];
}
template<size_t N,typename T>
constexpr const T& Perm<N,T>::operator[](size_t idx) const
{
    return this->X[idx];
}

template<size_t N, size_t L, typename T>
constexpr T& CArray<N,L,T>::operator[](size_t idx) 
{ 
    return this->xs[idx]; 
}
template<size_t N, size_t L, typename T>
constexpr const T& CArray<N,L,T>::operator[](size_t idx) const 
{ 
    return this->xs[idx]; 
}

template<size_t N, size_t L, typename T>
int CArray<N,L,T>::sum() const
{
    int s = 0;
    for(size_t i = 0; i < L; i++) s += xs[i];
    return s % N;
}

template<size_t N,typename T>
bool operator==(const Perm<N,T> &lhs, const Perm<N,T> &rhs)
{
    return lhs.X == rhs.X;
}

template<size_t N, size_t L, typename T>
bool operator==(const CArray<N,L,T>&lhs, const CArray<N,L,T>&rhs)
{
    return lhs.xs == rhs.xs;
}

/*!
 * @note we define permutation multiplication of P and Q as P * Q := P \comp Q;
 * it'll make pi: X * S_n -> X, (p, Q) -> p \otime Q := P * Q be a right action,
 * where P is such permutation othat P(id_X) = p. 
 */
template<size_t N, typename T>
constexpr Perm<N,T> operator*(const Perm<N,T> &X, const Perm<N,T> &Y)
{
    Perm<N,T> prod{};
    for(size_t i = 0; i < N; i++) prod[i] = X[Y[i]];
    return prod;
}

template<size_t N, typename T>
constexpr Perm<N,T> operator~(const Perm<N,T> &X)
{
    Perm<N,T> inv;
    for(size_t i = 0; i < N; i++) {
        inv[X[i]] = i;
    }
    return inv;
}

template<size_t N, size_t L, typename T>
constexpr CArray<N,L,T> operator*(const CArray<N,L,T> &xs, const Perm<L,T> &X)
{
    CArray<N,L,T> ys{};
    for(size_t i = 0; i < L; i++) ys[i] = xs[X[i]];
    return ys;
}

template<size_t N, size_t L, typename T>
constexpr CArray<N,L,T> operator+(const CArray<N,L,T> &xs, const CArray<N,L,T> &ys)
{
    CArray<N,L,T> prod{};
    for(size_t i = 0; i < L; i++) prod[i] = (xs[i] + ys[i]) % N;
    return prod;
}

template<size_t N, size_t L, typename T>
constexpr CArray<N,L,T> operator-(const CArray<N,L,T> &xs)
{
    CArray<N,L,T> inv{};
    for(size_t i = 0; i < L; i++) inv[i] = N - xs[i];
    return inv;
}