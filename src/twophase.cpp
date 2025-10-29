#include "twophase.hh"

const auto  &TM = SingletonTM<>::instance();
const auto  &TP = SingletonTP<>::instance();

/* for optimization
 * the continuation of TurnMoves A,B,C are dull (could be reduced) in cases like:
 *  - A=Ux1,B=Ux2,C     (A and its prev B are "homogeneous")
 *  - A=Ux1,B=Dx1,C=Ux1 (A and B are "disjoint" && A and C are "homogeneous")
 */
inline bool is_dull_triple(const TurnMove A, const int B, const int C) 
{
    return ( B>=Ux1 && B<=Bx3 ) &&
           ( (A/3==B/3) || ((C>=Ux1&&C<=Bx3) && A/3==C/3 && (3+A/3-B/3)%3==0) ) 
    ;
} 

template<TwoPhaseSolver::enum_phase I> 
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

template<TwoPhaseSolver::enum_phase I>
size_t TwoPhaseSolver::distance(const Coord &c)
{
    if constexpr (I == Ph1)
    return std::max((*TP.pTPSliceTwist)[c.slice][c.twist], 
                    (*TP.pTPSliceFlip)[c.slice][c.flip]);
    else 
    return std::max((*TP.pTPEdge4Corner)[c.edge4][c.corner], 
                    (*TP.pTPEdge4Edge8)[c.edge4][c.edge8]);
}

template<TwoPhaseSolver::enum_phase PhX> 
bool TwoPhaseSolver::search_phase(const Coord &c, size_t togo)
{
    if(togo == 0) return distance<PhX>(c) == 0;
    if(togo < distance<PhX>(c)) return false;
    
    for(auto m: EM<PhX>)
    {
        // assert(togo+1 < D);
        if(is_dull_triple(m,sofar_[PhX][togo],sofar_[PhX][togo+1])) continue;

        sofar_[PhX][togo-1] = m;
        bool ret = search_phase<PhX>(transform<PhX>(c,m), togo-1);

        // ret=true means we find a PhX solution within `togo` steps;
        // early exit is fine since there won't be a shorter PhX solution  
        // in iterative deepening search
        if(ret) return true;
    }
    return false;
}

Coord TwoPhaseSolver::ph2_origin_(Coord c) const
{
    // CubieCube transform: Coord::CubieCube2Coord(Coord::Coord2CubieCube(c) * ms);
    // Table transform optimization is working for corner only 
    int corner = c.corner, edge4, edge8;
    // auto cp = Coord::corner2cp(c.corner);
    auto ep = Coord::see2ep(c.slice,c.edge4,c.edge8);
    for(int i = rsolution_[Ph1].first-1; i>=0; --i) 
    { 
        auto m = static_cast<TurnMove>(rsolution_[Ph1].second[i]);
        corner = (*TM.pTMCorner)[m][corner];
        ep = ep * ElementaryMove[m].ep; 
    }
    edge4 = Coord::ep2edge4(ep);
    edge8 = Coord::ep2edge8(ep);    
    return Coord { 0,0,0,corner,edge4,edge8 };
}

auto TwoPhaseSolver::solve(const Coord &c, int step, bool best)
    -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>
{
    const int maxL = std::min(std::max(0,step),DS);     // largest length allowed
    int solL = maxL + 1;                                // smallest length found 
    std::array<std::vector<TurnMove>,2> solution;       // solution

    // reset sofar buffer: 
    // only once is enough since `set_ph_rsolution(d)` knows exact solution length d
    reset_ph_sofar_<Ph1>(); 
    reset_ph_sofar_<Ph2>();

    ///
    /// iterative deepening search 

    for(int d1 = distance<Ph1>(c); d1 <= maxL; d1++) 
    {
        // start Ph1 search
        bool ret1 = search_phase<Ph1>(c,d1);
        if(!ret1) continue;

        // Ph1 solution found
        set_ph_solution_<Ph1>(d1);

        // start Ph2 search
        auto c2 = ph2_origin_(c);
        int togo = solL - 1 - rsolution_[Ph1].first;
        for(int d2 = distance<Ph2>(c2); d2 <= togo; d2++)
        {
            bool ret2 = search_phase<Ph2>(c2,d2);
            if(!ret2) continue;

            // Ph2 solution found
            set_ph_solution_<Ph2>(d2);

            // save solution
            solution[Ph1] = get_ph_solution_<Ph1>();
            solution[Ph2] = get_ph_solution_<Ph2>();
            solL = solution[1].size() + solution[0].size();
            
            if(!best) goto found; 
            if(d2==0) goto found; else break;

            /* When a solution is found, logically we should continue from next layer-peer
               of Ph2 root, instead of the first node of layer under Ph1 root.
               That's why the solution is not optimal.
               We do not implement this since we tink it'll make the code 
               complicated and may involve ineffective searches. 
            */
        }
    }

    if(solL > maxL) 
        return std::make_tuple(false, std::vector<TurnMove>{}, std::vector<TurnMove>{});

    // solution found 
    found: 
    return std::make_tuple(true, solution[0], solution[1]);
}