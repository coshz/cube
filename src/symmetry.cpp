#include "symmetry.hh"
#include "data.hpp"
#include <array>
#include <cstdint>
#include <limits>
#include <memory>

namespace cube::symmetry {

void SymReduction::build_emb_corner(CornerDom& emb, const CornerConj& conj)
{
    static_assert(EQ_CORNER <= std::numeric_limits<uint16_t>::max());

    constexpr auto unvisited = corner_eq_t { std::numeric_limits<uint16_t>::max(),Id };
    emb.fill(unvisited);
    for(int c = 0, eq = -1; c < N_CORNER; ++c)
    {
        if(emb[c].eq != unvisited.eq) continue;
        emb[c] = { uint16_t(++eq), Id };
        for(Sym s: sym_entries)
        {
            if(s == Id) continue;
            auto &entry = emb[conj(s,c)];
            if(entry.eq == unvisited.eq) entry = { uint16_t(eq), s };
        }
    }
}

void SymReduction::build_rep_corner(EqCornerDom& rep, const CornerDom& emb)
{
    const auto unvisited = std::numeric_limits<uint16_t>::max();
    rep.fill(unvisited);
    for(int corner = 0; corner < N_CORNER; ++corner)
    {
        auto eq = emb[corner].eq;
        if(rep[eq] == unvisited) rep[eq] = corner;
    }
}

void SymReduction::build_emb_flipslice(FlipSliceDom& emb, const FlipSliceConj& conj)
{
    static_assert(EQ_FLIPSLICE <= std::numeric_limits<uint16_t>::max());

    constexpr auto unvisited = flipslice_eq_t { std::numeric_limits<uint16_t>::max(),Id };
    emb.fill(unvisited);
    for(int flip = 0, eq = -1; flip < N_FLIP; ++flip)
        for(int slice = 0; slice < N_SLICE; ++slice)
    {
        if(emb(flip,slice).eq != unvisited.eq) continue;
        emb(flip,slice) = { uint16_t(++eq), Id };
        for(auto s: sym_entries)
        {
            if(s == Id) continue;
            auto fs = conj(s,flip,slice);
            auto &entry = emb(fs[0],fs[1]);
            if(entry.eq == unvisited.eq) entry = { uint16_t(eq), s };
        }
    }
}

void SymReduction::build_rep_flipslice(EqFlipSliceDom& rep, const FlipSliceDom& emb)
{
    const auto unvisited = flipslice_t { std::numeric_limits<uint16_t>::max(),std::numeric_limits<uint16_t>::max() };
    rep.fill(unvisited);
    for(int flip = 0; flip < N_FLIP; ++flip)
        for(int slice = 0; slice < N_SLICE; ++slice)
        {
            auto eq = emb(flip,slice).eq;
            if(rep[eq] == unvisited) rep[eq] = { uint16_t(flip), uint16_t(slice) };
        }
}

void SymReduction::build_stab_corner(CornerStab& stab, const EqCornerDom& rep, const CornerConj& conj)
{
    for (int eq = 0; eq < EQ_CORNER; ++eq)
    {
        SymMask mask = 0;
        const auto c = rep[eq];
        for (Sym s : sym_entries)
        {
            if (conj[s][c] == c) mask |= SymMask(1u << s);
        }
        stab[eq] = mask;
    }
}

void SymReduction::build_stab_flipslice(FlipSliceStab& stab, const EqFlipSliceDom& rep, const FlipSliceConj& conj)
{
    for (int eq = 0; eq < EQ_FLIPSLICE; ++eq)
    {
        SymMask mask = 0;
        const auto fs = rep[eq];
        for (Sym s : sym_entries)
        {
            if (conj(s,fs[0],fs[1]) == fs) mask |= SymMask(1u << s);
        }
        stab[eq] = mask;
    }
}

} // namespace cube::pdb::symmetry
