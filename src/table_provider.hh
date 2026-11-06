#pragma once
#include "coord.hh"
#include "data.hpp"
#include "config.hh"
#include "symmetry.hh"
#include "table_view.hh"
#include <array>
#include <cassert>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#define TABLE_MAGIC 8677391316305539925
#define TABLE_VERSION 1

namespace cube::pdb::table {

namespace fs = std::filesystem;

template<typename> inline constexpr bool always_false_v = false;

enum class MoveKind { Twist, Flip, Slice, Edge4, Edge8, Corner, Count };
enum class ConjKind { TwistConj, SliceConj, Edge4Conj, Edge8Conj, CornerConj, FlipSliceConj, Count };
enum class EqKind   { RepCorner, RepFlipSlice, EmbCorner, EmbFlipSlice, Count };
enum class StabKind { CornerStab, FlipSliceStab, Count };
enum class PrunKind { SliceFlip, SliceTwist, Edge4Edge8, Edge4Corner, Edge8EqCorner, TwistEqFlipSlice, Count };

template<auto K> struct table_meta;
template<auto K> using table_t = typename table_meta<K>::type;
template<auto K> constexpr std::string_view tag_v = table_meta<K>::value;

#define Unwrap(...) __VA_ARGS__ 
#define spec_table_meta(Kind,Type,Tag) \
    template<> struct table_meta<Kind> \
    {                                  \
        using type = Unwrap Type;      \
        static constexpr std::string_view value = Tag; \
    };

spec_table_meta(MoveKind::Twist,        (MoveTableBase<N_TWIST>),       "TmTwist")
spec_table_meta(MoveKind::Flip,         (MoveTableBase<N_FLIP>),        "TmFlip")
spec_table_meta(MoveKind::Slice,        (MoveTableBase<N_SLICE>),       "TmSlice")
spec_table_meta(MoveKind::Edge4,        (MoveTableBase<N_EDGE4>),       "TmEdge4")
spec_table_meta(MoveKind::Edge8,        (MoveTableBase<N_EDGE8>),       "TmEdge8")
spec_table_meta(MoveKind::Corner,       (MoveTableBase<N_CORNER>),      "TmCorner")
spec_table_meta(ConjKind::TwistConj,    (ConjTableBase<N_TWIST>),       "TcTwist")
spec_table_meta(ConjKind::SliceConj,    (ConjTableBase<N_SLICE>),       "TcSlice")
spec_table_meta(ConjKind::Edge4Conj,    (ConjTableBase<N_EDGE4>),       "TcEdge4")
spec_table_meta(ConjKind::Edge8Conj,    (ConjTableBase<N_EDGE8>),       "TcEdge8")
spec_table_meta(ConjKind::CornerConj,   (ConjTableBase<N_CORNER>),      "TcCorner")
spec_table_meta(ConjKind::FlipSliceConj,(FlipSliceConj),                "TcFlipSlice")
spec_table_meta(EqKind::RepCorner,      (SymReduction::EqCornerDom),    "TeRepCorner")
spec_table_meta(EqKind::RepFlipSlice,   (SymReduction::EqFlipSliceDom), "TeRepFlipSlice")
spec_table_meta(EqKind::EmbCorner,      (SymReduction::CornerDom),      "TeEmbCorner")
spec_table_meta(EqKind::EmbFlipSlice,   (SymReduction::FlipSliceDom),   "TeEmbFlipSlice")
spec_table_meta(StabKind::CornerStab,   (SymReduction::CornerStab),     "TsCorner")
spec_table_meta(StabKind::FlipSliceStab,(SymReduction::FlipSliceStab),  "TsFlipSlice")
spec_table_meta(PrunKind::SliceFlip,        (PrunTableBase<N_SLICE,N_FLIP>),        "TpSliceFlip")
spec_table_meta(PrunKind::SliceTwist,       (PrunTableBase<N_SLICE,N_TWIST>),       "TpSliceTwist")
spec_table_meta(PrunKind::Edge4Edge8,       (PrunTableBase<N_EDGE4,N_EDGE8>),       "TpEdge4Edge8")
spec_table_meta(PrunKind::Edge4Corner,      (PrunTableBase<N_EDGE4,N_CORNER>),      "TpEdge4Corner")
spec_table_meta(PrunKind::Edge8EqCorner,    (PrunTableBase<N_EDGE8,EQ_CORNER>),     "TpEdge8EqCorner")
spec_table_meta(PrunKind::TwistEqFlipSlice, (PrunTableBase<N_TWIST,EQ_FLIPSLICE>),  "TpTwistEqFlipSlice")

#undef Unwrap
#undef spec_table_meta

template<size_t... Is> auto make_mk_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<MoveKind>(Is)>>...>;
template<size_t... Is> auto make_ck_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<ConjKind>(Is)>>...>;
template<size_t... Is> auto make_ek_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<EqKind>(Is)>>...>;
template<size_t... Is> auto make_sk_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<StabKind>(Is)>>...>;
template<size_t... Is> auto make_pk_tuple(std::index_sequence<Is...>) -> std::tuple<std::unique_ptr<table_t<static_cast<PrunKind>(Is)>>...>;

using MoveTables= decltype(make_mk_tuple(std::make_index_sequence<size_t(MoveKind::Count)>{}));
using ConjTables= decltype(make_ck_tuple(std::make_index_sequence<size_t(ConjKind::Count)>{}));
using EqTables  = decltype(make_ek_tuple(std::make_index_sequence<size_t(EqKind::Count)>{}));
using StabTables= decltype(make_sk_tuple(std::make_index_sequence<size_t(StabKind::Count)>{}));
using PrunTables= decltype(make_pk_tuple(std::make_index_sequence<size_t(PrunKind::Count)>{}));
using Tables    = decltype(std::tuple_cat(
    std::declval<MoveTables>(), 
    std::declval<ConjTables>(), 
    std::declval<EqTables>(), 
    std::declval<StabTables>(),
    std::declval<PrunTables>()
));

template<auto K>
constexpr size_t index_v = []() constexpr {
    using T = decltype(K);
    constexpr size_t C_Move = size_t(MoveKind::Count)
                    ,C_Conj = size_t(ConjKind::Count)
                    ,C_Eq   = size_t(EqKind::Count)
                    ,C_Stab = size_t(StabKind::Count);
    if constexpr (std::is_same_v<T, MoveKind>)  return size_t(K); 
    if constexpr (std::is_same_v<T, ConjKind>)  return size_t(K) + C_Move; 
    if constexpr (std::is_same_v<T, EqKind>)    return size_t(K) + C_Move + C_Conj; 
    if constexpr (std::is_same_v<T, StabKind>)  return size_t(K) + C_Move + C_Conj + C_Eq; 
    if constexpr (std::is_same_v<T, PrunKind>)  return size_t(K) + C_Move + C_Conj + C_Eq + C_Stab;
}();

class TablePool
{
    Tables tables_;
public:
    static const TablePool& instance() { return instance_mut(); }

    template<auto K> const auto& get() const 
    { 
        const auto& slot = std::get<index_v<K>>(tables_);
        if(!slot) {
            VTRACE("fatal: table %s is not initialized\n", tag_v<K>.data());
            std::terminate();
        }
        return *slot; 
    }

    /** @param Builder :: (table_t<K>&) -> void */
    template<auto K, typename Builder> const auto& ensure(Builder&& builder)
    {
        auto& slot = std::get<index_v<K>>(tables_);
        if(!slot) { 
            slot = std::make_unique<table_t<K>>();
            builder(*slot);
        }
        return *slot;
    }

    template<auto... K> void clean() { (std::get<index_v<K>>(tables_).reset(),...); }
private:
    friend class TableProvider;
    static TablePool& instance_mut() { static TablePool pool; return pool; }
}; 

enum class LoadStatus { Ok, Missing, Incompatible, Corrupted, OsErr };
template<auto K> struct table_with_header;

class TableProvider
{
    fs::path tdir_;
    TablePool& pool_;
public:
    template<bool use_symmetry>
    static const TableProvider& make(fs::path dir = {})
    {
        static TableProvider tp (
            dir.empty() ? cube::config::get_table_dir() : dir,
            std::bool_constant<use_symmetry>()
        );
        return tp;
    }

    template<auto K> [[nodiscard]] const auto& get() const { return pool_.get<K>(); }
private:
    template<bool use_symmetry>
    TableProvider(fs::path dir, std::bool_constant<use_symmetry>)
        : tdir_{dir} 
        , pool_{TablePool::instance_mut()}
    { load_or_build<use_symmetry>(); }

    template<bool use_symmetry>
    void load_or_build()
    {
        ensure_all<
            MoveKind::Twist, MoveKind::Flip, MoveKind::Slice,
            MoveKind::Edge4, MoveKind::Edge8, MoveKind::Corner
        >();

        if constexpr (!use_symmetry) {
            ensure_all<
                PrunKind::SliceFlip, PrunKind::SliceTwist, 
                PrunKind::Edge4Edge8, PrunKind::Edge4Corner
            >();
        } else {
            ensure_all<
                EqKind::EmbCorner, EqKind::EmbFlipSlice,
                ConjKind::TwistConj, ConjKind::Edge8Conj,
                PrunKind::TwistEqFlipSlice, 
                PrunKind::Edge4Edge8, PrunKind::Edge8EqCorner
            >();

            pool_.clean<
                ConjKind::CornerConj, ConjKind::FlipSliceConj,
                StabKind::CornerStab, StabKind::FlipSliceStab,
                EqKind::RepCorner, EqKind::RepFlipSlice
            >();
        }
    }

    template<auto K>
    static constexpr bool should_persist() 
    {
        using T = std::remove_cv_t<decltype(K)>;
        if constexpr (std::is_same_v<T, MoveKind> || std::is_same_v<T, PrunKind>) return true;
        else if constexpr (std::is_same_v<T, EqKind>) return K == EqKind::EmbCorner || K == EqKind::EmbFlipSlice;
        else if constexpr (std::is_same_v<T, ConjKind>) return K == ConjKind::TwistConj || K == ConjKind::Edge8Conj;
        else return false;
    }

    template<auto K> const auto& ensure()
    {
        return pool_.ensure<K>([this](table_t<K>& t){
            auto status = load_from<K>(t,tdir_);
            if(status == LoadStatus::Ok) return;
            if(!fs::exists(tdir_)) fs::create_directories(tdir_);
            if constexpr (std::is_same_v<decltype(K), PrunKind>) { VTRACE("[pdb] building pruning table %s...\n", tag_v<K>.data()); }
            build_table<K>(t);
            if constexpr (std::is_same_v<decltype(K), PrunKind>) { VTRACE("[pdb] done.\n"); }
            if constexpr (should_persist<K>()) save_to<K>(t,tdir_);
        });
    }

    template<auto... Ks> void ensure_all() { (ensure<Ks>(), ...); }

    /* build tables */

    template<MoveKind K> void build_table(table_t<K>& t) 
    {
        if constexpr (K == MoveKind::Twist)   { t.build(transform_twist); }
        if constexpr (K == MoveKind::Flip)    { t.build(transform_flip); }
        if constexpr (K == MoveKind::Slice)   { t.build(transform_slice); }
        if constexpr (K == MoveKind::Edge4)   { t.build(transform_edge4); }
        if constexpr (K == MoveKind::Edge8)   { t.build(transform_edge8); }
        if constexpr (K == MoveKind::Corner)  { t.build(transform_corner); }
    }

    template<ConjKind K> void build_table(table_t<K>& t) 
    {
        if constexpr (K == ConjKind::TwistConj)  { t.build(conjugate_twist); }
        if constexpr (K == ConjKind::SliceConj)  { t.build(conjugate_slice); }
        if constexpr (K == ConjKind::Edge4Conj)  { t.build(conjugate_edge4); }
        if constexpr (K == ConjKind::Edge8Conj)  { t.build(conjugate_edge8); }
        if constexpr (K == ConjKind::CornerConj) { t.build(conjugate_corner); }
        if constexpr (K == ConjKind::FlipSliceConj) { 
            for(int flip = 0; flip < N_FLIP; ++flip) {
                for(int slice = 0; slice < N_SLICE; ++slice) {
                    for(auto s: sym_entries)  
                        t[s][flip][slice] = conjugate_flipslice(s, {uint16_t(flip),uint16_t(slice)});
                }
            }
        }
    } 

    template<StabKind K> void build_table(table_t<K>& t) 
    {
        if constexpr(K == StabKind::CornerStab) { 
            SymReduction::build_stab_corner(t,
                ensure<EqKind::RepCorner>(), ensure<ConjKind::CornerConj>()
            );
        }
        if constexpr(K == StabKind::FlipSliceStab) { 
            SymReduction::build_stab_flipslice(t,
                ensure<EqKind::RepFlipSlice>(), ensure<ConjKind::FlipSliceConj>()
            );
        }
    }

    template<EqKind K> void build_table(table_t<K>& t) 
    {
        if constexpr (K == EqKind::EmbCorner)   { SymReduction::build_emb_corner(t, ensure<ConjKind::CornerConj>()); }
        if constexpr (K == EqKind::RepCorner)   { SymReduction::build_rep_corner(t, ensure<EqKind::EmbCorner>()); }
        if constexpr (K == EqKind::EmbFlipSlice){ SymReduction::build_emb_flipslice(t, ensure<ConjKind::FlipSliceConj>()); }
        if constexpr (K == EqKind::RepFlipSlice){ SymReduction::build_rep_flipslice(t, ensure<EqKind::EmbFlipSlice>()); }
    }

    template<PrunKind K> void build_table(table_t<K>& t)
    {
        if constexpr (K == PrunKind::SliceTwist)    { t.build_disjU(ensure<MoveKind::Slice>(), ensure<MoveKind::Twist>()); return; }
        if constexpr (K == PrunKind::SliceFlip)     { t.build_disjU(ensure<MoveKind::Slice>(), ensure<MoveKind::Flip>()); return ; }
        if constexpr (K == PrunKind::Edge4Corner)   { t.build_disjU(ensure<MoveKind::Edge4>(), ensure<MoveKind::Corner>()); return; }
        if constexpr (K == PrunKind::Edge4Edge8)    { t.build_disjU(ensure<MoveKind::Edge4>(), ensure<MoveKind::Edge8>()); return; }
        
        if constexpr (K == PrunKind::Edge8EqCorner) { 
            t.build_XxEqY(
                ensure<MoveKind::Edge8>(), ensure<ConjKind::Edge8Conj>(),
                ensure<MoveKind::Corner>(), ensure<EqKind::EmbCorner>(), ensure<EqKind::RepCorner>(), ensure<StabKind::CornerStab>()
            );
        }

        if constexpr (K == PrunKind::TwistEqFlipSlice) { 
            const auto& tran_flip = ensure<MoveKind::Flip>();
            const auto& tran_slice = ensure<MoveKind::Slice>();
            auto tran_flipslice = [&tran_flip,&tran_slice](TurnMove m, const flipslice_t& fs) -> flipslice_t {
                return { tran_flip(m,fs[0]), tran_slice(m,fs[1]) };
            };
            t.build_XxEqY(
                ensure<MoveKind::Twist>(), ensure<ConjKind::TwistConj>(),
                tran_flipslice,
                ensure<EqKind::EmbFlipSlice>(), ensure<EqKind::RepFlipSlice>(), ensure<StabKind::FlipSliceStab>()
            );
        }
    }

    /* dump / load Tables */

    template<auto K>
    static bool save_to(const table_t<K>& table, const fs::path& dir)
    {
        static_assert(std::is_trivially_copyable_v<table_t<K>>);
        std::ofstream f(dir / tag_v<K>, std::ios::binary);
        if(!f) return false;
        return table_with_header<K>::write(f, table);
    }

    template <auto K>
    static auto load_from(table_t<K>& table, const fs::path& dir) -> LoadStatus
    {
        static_assert(std::is_trivially_copyable_v<table_t<K>>);
        std::ifstream f(dir / tag_v<K>, std::ios::binary);
        if(!f) return LoadStatus::Missing;
        return table_with_header<K>::read(f, table);
    }
};

template<auto K>
struct table_with_header
{
    struct meta_info { uint64_t filetag, checksum; };

    static constexpr auto encode_filetag(std::array<uint32_t,2> vk) -> uint64_t
    { 
        const auto& [ver,kind] = vk;
        return TABLE_MAGIC ^ ((uint64_t(ver) << 32) | uint64_t(kind)); 
    }
    
    [[maybe_unused]] 
    static auto decode_filetag(uint64_t tag) -> std::array<uint32_t,2> 
    { 
        const uint64_t vk = tag ^ TABLE_MAGIC;
        return { uint32_t(vk >> 32), uint32_t(vk & 0xffffffff) }; 
    }

    static constexpr uint64_t filetag_v = encode_filetag({uint32_t(TABLE_VERSION),uint32_t(index_v<K>)});

    static auto read(std::ifstream& is, table_t<K>& t) -> LoadStatus
    {
        meta_info header{};
        if(!is.read(reinterpret_cast<char*>(&header), sizeof(header))) return LoadStatus::OsErr;
        if(!(header.filetag == filetag_v)) return LoadStatus::Incompatible;
        if(!is.read(reinterpret_cast<char*>(&t.storage), sizeof(t.storage))) return LoadStatus::OsErr;
        if(fnv1a(&t.storage, sizeof(t.storage)) != header.checksum) return LoadStatus::Corrupted;
        return LoadStatus::Ok;
    }

    static auto write(std::ofstream& os, const table_t<K>& t) -> bool
    {
        const auto header = meta_info{filetag_v, fnv1a(&t.storage, sizeof(t.storage))};
        os.write(reinterpret_cast<const char*>(&header), sizeof(header));
        os.write(reinterpret_cast<const char*>(&t.storage), sizeof(t.storage));
        return bool(os);
    }
            
private:
    static uint64_t fnv1a(const void* data, size_t n)
    {
        const auto* p = static_cast<const uint8_t*>(data);
        uint64_t h = 1469598103934665603ull;
        for(size_t i = 0; i < n; ++i) { h ^= p[i]; h *= 1099511628211ull; }
        return h;
    }
};
} // namespace cube::pdb::table