#pragma once
#include "def.h"
#include "coord.hh"
#include "cube.hh"
#include "table.hh"

#include <array>
#include <vector>
#include <tuple>

/*! 
 * @brief Kociemba's twophase algorithm
 */
class TwoPhaseSolver 
{
public:

    /*! 
     * @brief Attemp to solve `c` in `step` steps 
     * @param c the Coord of cube
     * @param step the max step to search
     * @param best try its best to find the short (but slower) solution 
     * @return (is_solved,sol1,sol2) 
     * @implements 
     * We adopt two iterative deepening searches in series rather than nesting 
     * them, as in original Kociemba's algorithm. This simplifies the 
     * implementation, but sacrifies the ease of extending the algorithm to 
     * find optimal solution. 
     * 
     * If `best` is false, the search will stop as soon as a solution is found;
     * otherwise, the search will continue to shorter solution is impossible.
     * Note the eventual solution is not guaranteed to be optimal.
     */
    auto solve(const Coord &c, int step, bool best) -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>;

protected:
    enum enum_phase { Ph1=0, Ph2=1 };
    
    /*! 
     * @brief The search algorithm in PhX 
     * @details 
     * The search tree is rooted at coord `c` and expands via moves in ElementaryMove, 
     * i.e. the coord of `m`-th child of node with coord `c` is `c * ElementaryMove[m]`; 
     * a node is considered a solution if its coord satisfies `distance<PHX>(c) == 0`. 
     *
     * During DFS traversal, the current move is always cached in the buffer at index `togo-1`; 
     * if a solution node is found, the buffer is flushed to the solution and return true;
     * otherwise, all nodes within depth `togo` are explored and return false. 
     */
    template<enum_phase PhX> bool search_phase(const Coord &c, size_t togo);

    /* move-table based coord transform */
    template<enum_phase PhX> static Coord transform(const Coord &c, const TurnMove &m);

    /* prunning-talbe based lower bound distance in phase 1/2 */
    template<enum_phase PhX> static size_t distance(const Coord &c);

    /* max search depth for phase 1/2 (conclusion from literatures) */
    static constexpr int D0 = 12, D1 = 18, DS = D0+D1;
    template<enum_phase PhX> static constexpr auto &D = std::get<PhX>(std::tie(D0,D1));

    /* elementary moves of two phases */
    static constexpr std::array<TurnMove,18> 
    EM0 = { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
    static constexpr std::array<TurnMove,10> 
    EM1 = { Ux1,Ux2,Ux3,Rx2,Fx2,Dx1,Dx2,Dx3,Lx2,Bx2 };
    template<enum_phase PhX> 
    static constexpr auto & EM = std::get<PhX>(std::tie(EM0,EM1));

private: 

    /* reset solution buffer */
    template<enum_phase PhX> 
    void reset_ph_sofar_()  { sofar_[PhX].fill(-1); }

    /* flush solution buffer to rsolution,rlen */
    template<enum_phase PhX>
    void set_ph_solution_(size_t L) 
    {
        rsolution_[PhX].first = L;
        std::copy(sofar_[PhX].begin(), sofar_[PhX].begin()+L, rsolution_[PhX].second.begin());
    }

    /* get solution of phase 1/2 by reversing rsolution */
    template<enum_phase PhX> auto get_ph_solution_() -> std::vector<TurnMove> 
    {
        int n = rsolution_[PhX].first;
        std::vector<TurnMove> sol(n);
        for(int i = n-1, j = 0; i >= 0; --i, j++) 
            sol[j] = static_cast<TurnMove>(rsolution_[PhX].second[i]);
        return sol;
    }

    /* the origin of phase 2, evaluated from phase 1 solution */
    Coord ph2_origin_(Coord c) const;

    std::array<std::array<int,DS+2>,2>                  sofar_;      // solution buffer
    std::array<std::pair<size_t,std::array<int,DS>>,2>  rsolution_;  // reverse of temp solution
};