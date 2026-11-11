#pragma once
#include "coord.hh"
#include "data.hpp"
#include "rubik.hh"
#include "table_provider.hh"
#include "config.hh"
#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

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
