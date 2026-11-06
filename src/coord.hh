#pragma once
#include "data.hpp"
#include "rubik.hh"
#include <cstdint>
#include <memory>

namespace cube::pdb {

/*!
 * @brief The coordinate space of cube. 
 * 
 * @details 
 *  Coord, the product space of (Twist,Flip,Slice,Corner,Edge4,Edge8), is used 
 * for tables generation. See CoordTables (in @ref table_coord.hh).
 *  The coordinates of:
 *  1. Ph1
 *      - twist: the orientation number of 8 corner cubies
 *      - flip: the orientation number of 12 edge cubies
 *      - slice: the location number of 4 UD-slice edge cubies (FR,FL,BL,BR)
 *  2. Ph2
 *      - corner: the location number of 8 corner cubies
 *      - edge4: the location number of 4 slice edge cubies
 *      - edge8: the location number of 8 non- UD-slice edge cubies
 * 
 * @remarks
 * Alought the three components (slice2ep,edge42ep,edge82ep) of \(s,e4,e8) -> ep
 * aren't well-defined functions, the following is guaranteed:
 *  - \(m, s)-> ep2slice( slice2ep(  s ) * m ) are well-defined
 *  - \(m,e4)-> ep2edge4( edge42ep( e4 ) * m ) are well-defined if s is fixed to 0
 *  - \(m,e8)-> ep2edge8( edge82ep( e8 ) * m ) are well-deinned if s is fixed to 0
 * where m in ElementaryMove.
 */
struct Coord
{
    /*! Phase 1 coordinate
     * @param twist: the orientation number of 8 corner cubies
     * @param flip: the orientation number of 12 edge cubies
     * @param slice: the location number of 4 slice edge cubies (FR,FL,BL,BR)
     */
    int twist, flip, slice;

    /*! Phase 2 coordinate
     * @param corner: the location number of 8 corner cubies
     * @param edge4: the location number of 4 slice edge cubies
     * @param edge8: the location number of 8 non-slice edge cubies
     */
    int corner, edge4, edge8;

    static const Coord id;

    /* auxiliary conversion functions */

    static CornerOri    twist2co(int);
    static int          co2twist(const CornerOri &);

    static EdgeOri      flip2eo(int);
    static int          eo2flip(const EdgeOri &);

    static CornerPerm   corner2cp(int);
    static int          cp2corner(const CornerPerm &);

    static int          ep2slice(const EdgePerm &);
    static int          ep2edge4(const EdgePerm &);
    static int          ep2edge8(const EdgePerm &);
    static EdgePerm     slice2ep(int);        // incomplete EdgePerm
    static EdgePerm     edge42ep(int);        // incomplete EdgePerm
    static EdgePerm     edge82ep(int);        // incomplete EdgePerm
    static EdgePerm     see2ep(int,int,int);  // complete EdgePerm
        
    /* conversion between Coord and CubieCube */

    static Coord        CubieCube2Coord(const CubieCube &);
    static CubieCube    Coord2CubieCube(const Coord &);
};

constexpr bool operator==(const Coord &c1, const Coord &c2)
{
    return c1.twist == c2.twist && c1.flip == c2.flip && c1.slice == c2.slice
        && c1.corner == c2.corner && c1.edge4 == c2.edge4 && c1.edge8 == c2.edge8;
}

inline constexpr Coord Coord::id = { 0,0,0,0,0,0 };

/* type of TurnMove-based transformation tables  */

template<size_t coord_size> 
using MoveTableLayout = NArray<uint16_t,N_MOVE,coord_size>;

using MoveTwist = MoveTableLayout<N_TWIST>;
using MoveFlip  = MoveTableLayout<N_FLIP>;
using MoveSlice = MoveTableLayout<N_SLICE>;
using MoveEdge4 = MoveTableLayout<N_EDGE4>;
using MoveEdge8 = MoveTableLayout<N_EDGE8>;
using MoveCorner= MoveTableLayout<N_CORNER>;

/** 
 * @defgroup TurnMove-transformation generators 
 * @{ 
 */

inline int transform_twist(TurnMove m, int twist)   { return Coord::co2twist(Coord::twist2co(twist) * ElementaryMove[m]); }
inline int transform_flip(TurnMove m, int flip)     { return Coord::eo2flip(Coord::flip2eo(flip) * ElementaryMove[m]); }
inline int transform_slice(TurnMove m, int slice)   { return Coord::ep2slice(Coord::slice2ep(slice) * ElementaryMove[m]); }
inline int transform_corner(TurnMove m, int corner) { return Coord::cp2corner(Coord::corner2cp(corner) * ElementaryMove[m]); }
inline int transform_edge4(TurnMove m, int edge4)   { return Coord::ep2edge4(Coord::edge42ep(edge4) * ElementaryMove[m]); }
inline int transform_edge8(TurnMove m, int edge8)   { return Coord::ep2edge8(Coord::edge82ep(edge8) * ElementaryMove[m]); }

/** @} */

} // namespace cube::pdb