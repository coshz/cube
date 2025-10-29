#include "table.hh"
#include "coord.hh"
#include "utils.hpp"
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

static fs::path table_dir_fallback(std::string dir) 
{
    if(dir != "") return fs::path(dir);
    try {
        return get_cache_dir() / "cube" / "tables";
    } catch (...) {
#ifdef INSTALL_PREFIX
    return fs::path(INSTALL_PREFIX) / "cube" / "tables";
#else 
    return fs::current_path() / "tables";
#endif
    }
}

template <typename Table>
void save_to(const Table &table, std::string path)
{
    std::ofstream f(path, std::ios::binary);
    if(f.is_open()) {
        f.write(reinterpret_cast<const char*>(&table.data), sizeof(table.data));
        f.close();
    }
}

template <typename Table>
void load_from(Table &table, std::string path)
{
    VPRINT("loading table from %s...", path.c_str());
    std::ifstream f(path,std::ios::binary);
    f.read(reinterpret_cast<char*>(&table.data), sizeof(table.data));
    f.close();
    VPRINT("done.\n");
}

template<typename T>
template<typename Table, typename F1, typename F2>
std::enable_if_t<Table::shape[0] == N_MOVE>
TableMove<T>::buildMoveTable(Table &t, F1&& coord2i, F2&& i2coord, std::string filename)
{
    VPRINT("creating move table %s of shape (%zu,%zu)... ", 
           filename.c_str(), t.shape[0], t.shape[1]);
    for(size_t i = 0; i < t.shape[0]; i++) for(size_t j = 0; j < t.shape[1]; j++) {
        t[i][j] = coord2i(i2coord(j) * ElementaryMove[i]);
    }
    if(filename != "") save_to(t, tdir/filename);
    VPRINT("done.\n");
}

template<typename T>
TableMove<T>::TableMove(std::string dir)
:tdir(table_dir_fallback(dir))
{
    VPRINT("INIT MOVE TABLES -- \n");
    pTMTwist     = new NArray<T,N_MOVE,N_TWIST>;
    pTMFlip      = new NArray<T,N_MOVE,N_FLIP>;
    pTMSlice     = new NArray<T,N_MOVE,N_SLICE>;
    pTMCorner    = new NArray<T,N_MOVE,N_CORNER>;
    pTMEdge4     = new NArray<T,N_MOVE,N_EDGE4>;
    pTMEdge8     = new NArray<T,N_MOVE,N_EDGE8>;

    if(!fs::exists(tdir/"tm_twist.dat")) {
        if(!fs::exists(tdir)) fs::create_directories(tdir);
        buildMoveTable(*pTMTwist, Coord::co2twist, Coord::twist2co, "tm_twist.dat");
        buildMoveTable(*pTMFlip, Coord::eo2flip, Coord::flip2eo, "tm_flip.dat");
        buildMoveTable(*pTMSlice, Coord::ep2slice, Coord::slice2ep, "tm_slice.dat");
        buildMoveTable(*pTMCorner, Coord::cp2corner, Coord::corner2cp, "tm_corner.dat");
        buildMoveTable(*pTMEdge4, Coord::ep2edge4, Coord::edge42ep, "tm_edge4.dat");
        buildMoveTable(*pTMEdge8, Coord::ep2edge8, Coord::edge82ep, "tm_edge8.dat");
    } else {
        load_from(*pTMTwist, tdir/"tm_twist.dat");
        load_from(*pTMFlip, tdir/"tm_flip.dat");
        load_from(*pTMSlice, tdir/"tm_slice.dat");
        load_from(*pTMCorner, tdir/"tm_corner.dat");
        load_from(*pTMEdge4, tdir/"tm_edge4.dat");
        load_from(*pTMEdge8, tdir/"tm_edge8.dat");
    }
    VPRINT("-- DONE.\n");
}

template<typename T>
TableMove<T>::~TableMove()
{
    delete pTMTwist;
    delete pTMFlip;
    delete pTMSlice;
    delete pTMCorner;
    delete pTMEdge4;
    delete pTMEdge8;
}

template<typename T>
template<typename Table, typename MT1, typename MT2>
std::enable_if_t<Table::shape[0] == MT1::shape[1] && Table::shape[1] == MT2::shape[1]>
TablePrunning<T>::buildPrunningTable(
    Table &t, const MT1 &mt1, const MT2 &mt2, std::string filename)
{
    VPRINT("creating prunning table %s of shape (%zu,%zu):\n", 
           filename.c_str(), mt1.shape[1], mt2.shape[1]);
    std::fill_n(&t.data[0][0], t.size, (typename Table::value_type) ~0UL);
    t[0][0] = 0;

    typename Table::value_type depth = 0;
    size_t count = 1;
    VPRINT("\tdepth %2d: %10zu / %-10zu.\n", depth, count, t.size);
    while(count < t.size)
    {
        for(size_t i = 0; i < t.shape[0]; i++)
        for(size_t j = 0; j < t.shape[1]; j++)
        if(t[i][j] == depth) {
            for(auto k = 0; k < N_MOVE; k++) {
                auto ii = mt1[k][i], jj = mt2[k][j];
                if(t[ii][jj] == (typename Table::value_type)~0UL) { t[ii][jj] = depth + 1; count++; }
            } 
        }
        depth++;
        VPRINT("\tdepth %2d: %10zu / %-10zu.\n", depth, count, t.size);
    }
    if(filename != "") save_to(t, tdir/filename);
    VPRINT("done.\n");
}

template<typename T>
TablePrunning<T>::TablePrunning(std::string dir)
:tdir(table_dir_fallback(dir))
{
    VPRINT("INIT PRUNNING TABLES -- \n");
    pTPSliceFlip     = new NArray<T,N_SLICE,N_FLIP>;
    pTPSliceTwist    = new NArray<T,N_SLICE,N_TWIST>;
    pTPEdge4Edge8    = new NArray<T,N_EDGE4,N_EDGE8>;
    pTPEdge4Corner   = new NArray<T,N_EDGE4,N_CORNER>;

    if(!fs::exists(tdir/"tp_slicetwist.dat")) {
        const auto &TM = SingletonTM<>::instance();
        buildPrunningTable(*pTPSliceTwist, *TM.pTMSlice, *TM.pTMTwist, "tp_slicetwist.dat");
        buildPrunningTable(*pTPSliceFlip, *TM.pTMSlice, *TM.pTMFlip, "tp_sliceflip.dat");
        buildPrunningTable(*pTPEdge4Corner, *TM.pTMEdge4, *TM.pTMCorner, "tp_edge4corner.dat");
        buildPrunningTable(*pTPEdge4Edge8, *TM.pTMEdge4, *TM.pTMEdge8, "tp_edge4edge8.dat");
    } else {
        load_from(*pTPSliceTwist, tdir/"tp_slicetwist.dat");
        load_from(*pTPSliceFlip, tdir/"tp_sliceflip.dat");
        load_from(*pTPEdge4Corner,tdir/"tp_edge4corner.dat");
        load_from(*pTPEdge4Edge8, tdir/"tp_edge4edge8.dat");
    }
    VPRINT("-- DONE.\n");
}

template<typename T>
TablePrunning<T>::~TablePrunning()
{
    delete pTPSliceFlip;
    delete pTPSliceTwist;
    delete pTPEdge4Edge8;
    delete pTPEdge4Corner;
}

template struct TableMove<>;
template struct TablePrunning<>;