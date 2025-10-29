#include "twophase.hh"

const TableMove<>       &TM = SingletonTM<>::getInstance().getTable();
const TablePrunning<>   &TP = SingletonTP<>::getInstance().getTable();

bool isValidPair(const TurnMove &m1, const TurnMove &m2) { 
    return m1/3 != m2/3 && m1/3 != 3 + m2/3; 
}

template<TwoPhaseSolver::enum_phase I> 
Coord TwoPhaseSolver::transform(const Coord &c, const TurnMove &m)
{
    if constexpr (I == Ph1)
    return Coord {
        (*TM.pTMTwist)[m][c.twist], (*TM.pTMFlip)[m][c.flip], (*TM.pTMSlice)[m][c.slice],
        -1,-1,-1
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

template<TwoPhaseSolver::enum_phase I>
bool TwoPhaseSolver::isInSubG(const Coord &c)
{
/*
    if constexpr (I == Ph1) 
        return c.twist == 0 && c.flip == 0 && c.slice == 0;
    else 
        return c == Coord::id;
*/
    // better performance 
    return distance<I>(c) == 0; 
}

template<TwoPhaseSolver::enum_phase I>
auto TwoPhaseSolver::get_solution_ph() const -> std::vector<TurnMove> 
{
    auto sol_pre(sofar_[I]);
    std::vector<TurnMove> sol;
    std::for_each(sol_pre.rbegin(), sol_pre.rend(),
        [&sol](auto &m) { if(m >= 0) sol.emplace_back(static_cast<TurnMove>(m)); }
    );
    return sol;
}

auto TwoPhaseSolver::solve(const Coord &c, size_t max_step) 
-> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>
{
    cube_ = c;
    exit_[0] = exit_[1] = false;
    max_len = max_step, cur_len = max_step + 1;
    for(auto i = distance<Ph1>(c); i < depth_ph1 && !exit_[Ph1]; i++) 
    {
        exit_[Ph2] = false;
        sofar_[Ph1].fill(~0),sofar_[Ph2].fill(~0);     // reset sofar
        search_phase1(c,i);
    }

    bool found = (cur_len > max_len) ? false : true;
    return std::make_tuple(found, solution_[Ph1], solution_[Ph2]);
}

void TwoPhaseSolver::search_phase1(const Coord &c1, size_t togo)
{
    if(togo == 0) {
    /* Move on next search_phase1, or start search_phase2 */

        if(!isInSubG<Ph1>(c1)) return; 

        auto s1 = this->get_solution_ph<Ph1>();
        auto c2 = [](const CubieCube &cc, const std::vector<TurnMove> &s) {
            auto cp = cc.cp;
            auto ep = cc.ep;
            for(auto &m: s) {
                cp = cp * ElementaryMove[m];
                ep = ep * ElementaryMove[m];
            }
            return Coord {
                0,0,0, 
                Coord::cp2corner(cp),Coord::ep2edge4(ep),Coord::ep2edge8(ep)
            };
        }(Coord::Coord2CubieCube(cube_), s1);

        for(auto i = distance<Ph2>(c2);
            i < std::min(depth_ph2, cur_len-s1.size()) && !exit_[Ph2]; i++) 
        {
            search_phase2(c2,i);
        }
    } 
    else
    for(auto &m: EM0)
    {
        if(sofar_[Ph1][togo] != ~0  && 
          !isValidPair(m,static_cast<TurnMove>(sofar_[Ph1][togo]))
        ) continue;

        auto c1_update = transform<Ph1>(c1, m);      
        auto d = distance<Ph1>(c1_update);

        if(d < togo) {
            sofar_[Ph1][togo-1] = m;
            search_phase1(c1_update, togo-1);
        }
    }
}

void TwoPhaseSolver::search_phase2(const Coord &c2, size_t togo)
{
    if(togo == 0) { 
        if(!isInSubG<Ph2>(c2)) return;

        auto sol1 = get_solution_ph<Ph1>();
        auto sol2 = get_solution_ph<Ph2>();
        auto len = sol1.size() + sol2.size();

        if(cur_len > len) {
            cur_len = len;
            std::copy(sol1.begin(),sol1.end(),std::back_inserter(solution_[0]));
            std::copy(sol2.begin(),sol2.end(),std::back_inserter(solution_[1]));

            exit_[Ph1] = (cur_len <= max_len) ? true : false;
            exit_[Ph2] = true; 
        }
        return; 
    }
    for(auto m: EM1)
    {
        if(sofar_[Ph2][togo] != ~0 && 
           !isValidPair(m,static_cast<TurnMove>(sofar_[Ph2][togo]))
        ) continue;

        auto c2_update = transform<Ph2>(c2, m);
        auto d = distance<Ph2>(c2_update);

        if(d < togo) {
            sofar_[Ph2][togo-1] = m;
            search_phase2(c2_update, togo-1);
        }
    }
}