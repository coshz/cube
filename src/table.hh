#pragma once
#include "def.h"
#include "help.hpp"
#include "coord.hh"

#include <fstream>
#include <filesystem>
#include <type_traits>
#include <limits>

typedef uint16_t    default_mt_value_t;
typedef uint8_t     default_pt_value_t;

template<typename T> struct TableMove;
template<typename T> struct TablePrunning;

/* dump / load Tables */
template <typename Table> void save_to(const Table &table, std::string path);
template <typename Table> void load_from(Table &table, std::string path);

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

template<typename T=default_mt_value_t>
using SingletonTM = Singleton<TableMove<T>>;

template<typename T=default_pt_value_t>
using SingletonTP = Singleton<TablePrunning<T>>;

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
template<typename T=default_mt_value_t>
struct TableMove
{
    static_assert(std::is_integral<T>::value);
    static_assert(std::numeric_limits<T>::digits >= 16);

    using value_t = T;
    TableMove(std::string dir="");
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
template<typename T=default_pt_value_t>
struct TablePrunning
{
    using value_type = T;
    TablePrunning(std::string dir="");
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

/* symmetry table 
d(s^-1*x*s,1) = d(x,1), s in S.
*/
