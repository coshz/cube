#pragma once
#include "data.hpp"
#include "coord.hh"
#include "rubik.hh"
#include <array>
#include <cstdint>
#include <memory>
#include <utility>

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
