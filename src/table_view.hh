#pragma once 
#include "coord.hh"
#include "data.hpp"
#include "rubik.hh"
#include "symmetry.hh"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <queue>
#include <type_traits>

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