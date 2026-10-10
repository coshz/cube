#include "twophase.hh"
#include "rubik.hh"
#include "table.hh"
#include "coord.hh"
#include "data.hpp"

#include <algorithm>
#include <array>
#include <tuple>
#include <vector>

namespace cube::solver {

#define TM cube::pdb::get_TM()
#define TP cube::pdb::get_TP()

/* important optimization
 * the continuation of TurnMoves A and its prev B could be reduced in cases:
 *  - A=Ux1,B=Ux2     (A and its prev B are "homogeneous")
 *  - A=Dx1,B=Ux1     (A > B && A and B are "disjoint")
 */
inline bool is_dull_pair(const TurnMove A, const int B)
{
    if(!(B >= Ux1 && B <= Bx3)) return false;
    return (A/3 == B/3) || (A/3 == B/3+3);
}

template<enum_phase I>
Coord TwoPhaseSolver::transform(const Coord &c, const TurnMove &m)
{
    if constexpr (I == Ph1)
    return Coord {
        (*TM.pTMTwist)[m][c.twist], (*TM.pTMFlip)[m][c.flip], (*TM.pTMSlice)[m][c.slice],
        -1,-1,-1 /* -1: not used */
    }; 
    else
    return Coord {
        0,0,0,
        (*TM.pTMCorner)[m][c.corner], (*TM.pTMEdge4)[m][c.edge4], (*TM.pTMEdge8)[m][c.edge8]
    };
}

template<enum_phase I>
size_t TwoPhaseSolver::distance(const Coord &c)
{
    if constexpr (I == Ph1)
    return std::max((*TP.pTPSliceTwist)[c.slice][c.twist],
                    (*TP.pTPSliceFlip)[c.slice][c.flip]);
    else
    return std::max((*TP.pTPEdge4Corner)[c.edge4][c.corner],
                    (*TP.pTPEdge4Edge8)[c.edge4][c.edge8]);
}

template<enum_phase PhX>
bool TwoPhaseSolver::search_phase(const Coord &c, size_t togo)
{
    if(togo < distance<PhX>(c)) return false;
    if(togo == 0) return distance<PhX>(c) == 0;
   
    for(auto m: EM<PhX>)
    {
        if(is_dull_pair(m,get_ph_sofar<PhX>(togo))) continue;

        set_ph_sofar<PhX>(togo-1,m);
        bool ret = search_phase<PhX>(transform<PhX>(c,m), togo-1);

        // ret=true means we find a PhX solution within `togo` steps;
        // early exit is fine since there won't be a shorter PhX solution 
        // in iterative deepening search
        if(ret) return true;
    }
    return false;
}

Coord TwoPhaseSolver::ph2_origin(Coord c) const
{
    // CubieCube transform: Coord::CubieCube2Coord(Coord::Coord2CubieCube(c) * ms);
    // （ Table transform optimization is working for corner only ）
    int corner = c.corner, edge4, edge8;
    // auto cp = Coord::corner2cp(c.corner);
    auto ep = Coord::see2ep(c.slice,c.edge4,c.edge8);
    for(auto &m : get_ph_solution<Ph1>())
    {
        corner = (*TM.pTMCorner)[m][corner];
        ep = ep * ElementaryMove[m].ep;
    }
    edge4 = Coord::ep2edge4(ep);
    edge8 = Coord::ep2edge8(ep);   
    return Coord { 0,0,0,corner,edge4,edge8 };
}

auto TwoPhaseSolver::solve(const Coord &c, int step)
    -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>
{
    // reset sofar buffer:
    reset_ph_sofar<Ph1>();
    reset_ph_sofar<Ph2>();

    /// iterative deepening search

    const size_t maxL = std::clamp(step,0,DS);
    for(auto d1 = distance<Ph1>(c); d1 <= maxL; d1++)
    {
        // start Ph1 search
        bool ret1 = search_phase<Ph1>(c,d1);
        if(!ret1) continue;

        // Ph1 solution found
        set_ph_solution<Ph1>(d1);

        // start Ph2 search
        auto c2 = ph2_origin(c);
        for(auto d2 = distance<Ph2>(c2); d2 <= maxL-d1; d2++)
        {
            bool ret2 = search_phase<Ph2>(c2,d2);
            if(!ret2) continue;

            // Ph2 solution found
            set_ph_solution<Ph2>(d2);
            return {true, get_ph_solution<Ph1>(), get_ph_solution<Ph2>()};
        }
    }
    return {false, {}, {}};
}

auto TwoPhaseOptimSolver::solve(const Coord &c, size_t good)
    -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>
{
    good_ = good;
    root_ = c;

    size_t bestL = DS + 1;

    reset_ph_sofar<Ph1>();
    for(auto togo = distance<Ph1>(c); togo < bestL; ++togo)
    {
        int l = start_search_ph1(c,togo,togo,bestL-1);
        if(l >= 0) bestL = std::min(bestL,static_cast<size_t>(l));
        if(bestL <= good_) break;
    }
    if(bestL > DS) return {false, {}, {}};
    else return { true, get_ph_solution<Ph1>(), get_ph_solution<Ph2>() };
}

int TwoPhaseOptimSolver::start_search_ph1(const Coord& c, size_t d, size_t d_orig, size_t maxL)
{
    if(d < distance<Ph1>(c)) return -1;
    if(d == 0) {
        set_ph_solution<Ph1>(d_orig);
        auto c2 = ph2_origin(root_);
        reset_ph_sofar<Ph2>();
        for(auto togo = distance<Ph2>(c2); togo <= maxL; ++togo)
        {
            bool succ = start_search_ph2(c2,togo);
            if(succ) {
                set_ph_solution<Ph2>(togo);
                return togo;
            }
        }
        return -1;
    }

    int bestL = maxL + 1;
    for(auto m: EM<Ph1>) 
    {
        if(is_dull_pair(m, get_ph_sofar<Ph1>(d))) continue;
        
        set_ph_sofar<Ph1>(d-1,m);
        auto l = start_search_ph1(transform<Ph1>(c,m), d-1, d_orig, bestL-2);
        if(l >= 0 && bestL > l+1) {
            bestL = l+1;
        }
        if(bestL + d_orig - d <= good_) break; // current is good enough
        if(d >= bestL) break; // no shorter solution
    }
    return bestL > static_cast<int>(maxL) ? -1 : bestL;
}

bool TwoPhaseOptimSolver::start_search_ph2(const Coord& c, size_t togo)
{   if(togo < distance<Ph2>(c)) return false;
    if(togo == 0) return distance<Ph2>(c) == 0;
    for(auto m: EM<Ph2>) 
    {
        if(is_dull_pair(m, get_ph_sofar<Ph2>(togo))) continue;
        
        set_ph_sofar<Ph2>(togo-1,m);
        bool ret = start_search_ph2(transform<Ph2>(c,m), togo-1);
        if(ret) return true;
    }
    return false;
}

} // namespace cube::solver