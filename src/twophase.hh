#pragma once
#include "def.h"
#include "coord.hh"
#include "cube.hh"
#include "table.hh"

#include <array>
#include <vector>
#include <tuple>

/*! 
 * @brief The Kociemba's twophase algorithm
 */
class TwoPhaseSolver 
{
public:

    /*! 
     * @brief Attempt to solve `c` in `step` steps 
     * @param c the Coord of cube
     * @param step the maximum steps allowed for search
     * @param best whether to search exhaustively for the shortest solution
     * @return (is_solved,sol1,sol2) 
     * @note 
     * We adopt two iterative deepening searches in series rather than nesting 
     * them, as in the original Kociemba algorithm. This simplifies the 
     * implementation, but sacrifices the ease of extending the algorithm to 
     * find the two-phase optimum. (Btw, the two-phase optimal solution is 
     * intrinsically not guaranteed to be theoretically optimal.)
     */
    auto solve(const Coord &c, int step, bool best) 
        -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>;

protected:
    enum enum_phase { Ph1=0, Ph2=1 };
    
    /*! 
     * @brief The search algorithm in PhX 
     * @param c the Coord of cube
     * @param togo the maximum search depth 
     * @return `true` if a solution is found within depth `togo`
     * 
     * @details The search tree is rooted at coord `c` and expands via ElementaryMove, 
     * i.e., the `m`-th child of node `c` is `c * ElementaryMove[m]`. 
     * During DFS traversal, the current move is always cached in buffer `sofar_` 
     * at index `togo-1`. If a solution node is reached, the buffer is flushed 
     * into `rsolution_` and return true. Otherwise, it returns false after all
     * nodes up to depth `togo` have been explored.
     * A node `x` is considered a solution if `distance<PHX>(x) == 0`. 
     */ 
    template<enum_phase PhX> bool search_phase(const Coord &c, size_t togo);

    /* move-table based coord transform */
    template<enum_phase PhX> static Coord transform(const Coord &c, const TurnMove &m);

    /* prunning-talbe based lower bound distance in phase 1/2 */
    template<enum_phase PhX> static size_t distance(const Coord &c);

    /* max search depth for phase 1/2 (conclusion from literatures) */
    static constexpr int D0 = 12, D1 = 18, DS = D0+D1;
    template<enum_phase PhX> static constexpr auto& D = std::get<PhX>(std::tie(D0,D1));

    /* elementary moves of two phases */
    static constexpr std::array<TurnMove,18> EM0 
        = { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
    static constexpr std::array<TurnMove,10> EM1 
        = { Ux1,Ux2,Ux3,Rx2,Fx2,Dx1,Dx2,Dx3,Lx2,Bx2 };
    template<enum_phase PhX> 
    static constexpr auto& EM = std::get<PhX>(std::tie(EM0,EM1));

private: 

    /* reset solution buffer */
    template<enum_phase PhX> 
    void reset_ph_sofar_()  { sofar_[PhX].fill(-1); }

    /* flush solution buffer to rsolution ( 0 <= L <= DS ) */
    template<enum_phase PhX>
    void set_ph_solution_(size_t L) 
    {
        rsolution_[PhX].first = L;
        std::copy(sofar_[PhX].begin(), sofar_[PhX].begin()+L, rsolution_[PhX].second.begin());
    }

    /* get solution of phase 1/2 by reversing rsolution */
    template<enum_phase PhX> 
    auto get_ph_solution_() const -> std::vector<TurnMove> 
    {
        size_t n = rsolution_[PhX].first;
        if(n == 0) return {};
        std::vector<TurnMove> sol(n);
        for(auto i = 0; i < n; i++) {
            sol[i] = static_cast<TurnMove>(rsolution_[PhX].second[n-1-i]);
        }
        return sol;
    }

    /* the origin of phase 2, evaluated from phase 1 solution */
    Coord ph2_origin_(Coord c) const;

    std::array<std::array<int,DS+2>,2>                  sofar_;      // solution buffer
    std::array<std::pair<size_t,std::array<int,DS>>,2>  rsolution_;  // reverse of solution
};