#pragma once
#include "coord.hh"
#include "data.hpp"

#include <algorithm>
#include <array>
#include <utility>
#include <vector>
#include <tuple>

namespace cube::solver {
using namespace cube::data;
using namespace cube::pdb;

/* The search phase */
enum enum_phase { Ph1=0, Ph2=1 };

struct solve_session 
{
    const Coord root;
    const size_t good_enough;
    const size_t max_step;
    // solve_session(const Coord &c, size_t ge) : root(c), good_enough(ge) {}
};

/*!
 * @brief The Kociemba's twophase algorithm
 */
class TwoPhaseSolver
{
public:
    /*!
     * @brief Attempt to the first solution to `c` within `max_step` steps
     * @param c the Coord of cube
     * @param max_step the maximum steps allowed for search
     * @return (is_solved,sol1,sol2) - solN means solution in phase N
     * @remark
     * We adopt two iterative deepening searches in series rather than nesting
     * them, as in the original Kociemba algorithm. This simplifies the
     * implementation, but sacrifices the ease of extending the algorithm to
     * find the two-phase optimum. 
     */
    auto solve(const Coord &c, int max_step = DS)
        -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>;

protected:
    /*!
     * @brief The search algorithm in PhX
     *
     * The search tree is rooted at coord `c` and expands via ElementaryMove,
     * i.e., the `m`-th child of node `c` is `c * ElementaryMove[m]`.
     * During DFS traversal, the current move is always cached in buffer `sofar_`
     * at index `togo-1`. If a solution node is reached, the buffer is flushed
     * into `rsolution_` and return true. Otherwise, it returns false after all
     * nodes up to depth `togo` have been explored.
     * A node `x` is considered a solution if `distance<PhX>(x) == 0`.
     * 
     * @param c     the Coord of cube
     * @param togo  the exact search depth
     * @return      `true` if a solution is found
     */
    template<enum_phase PhX> 
    bool search_phase(const Coord &c, size_t togo);

    /** 
     * @brief move-table based coord transform 
     * @attention In the current implementation, the Ph2 components of resultant
     * are left uncomputed in Ph1. Therefore, please DO NOT use this function to 
     * obtain the starting coord for Ph2. Use @ref `ph2_origin` instead. 
     */
    template<enum_phase PhX> 
    static Coord transform(const Coord &c, const TurnMove &m);

    /* prunning-talbe based lower bound distance in phase 1/2 */
    template<enum_phase PhX> 
    static size_t distance(const Coord &c);

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

protected:

    /* reset solution buffer */
    template<enum_phase PhX>
    void reset_ph_sofar() { sofar_[PhX].fill(-1); }

    /* cache the move of current step */
    template<enum_phase PhX>
    void set_ph_sofar(int step, int move) { sofar_[PhX][step] = move; }

    /* */
    template<enum_phase PhX>
    int get_ph_sofar(int step) { return sofar_[PhX][step]; }

    /* flush solution buffer to rsolution ( 0 <= L <= DS ) */
    template<enum_phase PhX>
    void set_ph_solution(size_t L)
    {
        rsolution_[PhX].first = L;
        std::copy(sofar_[PhX].begin(), sofar_[PhX].begin()+L, rsolution_[PhX].second.begin());
    }

    /* get solution of phase 1/2 by reversing rsolution */
    template<enum_phase PhX>
    auto get_ph_solution() const -> std::vector<TurnMove>
    {
        size_t n = rsolution_[PhX].first;
        if(n == 0) return {};
        std::vector<TurnMove> sol(n);
        for(auto i = 0; i < n; i++) {
            sol[i] = static_cast<TurnMove>(rsolution_[PhX].second[n-1-i]);
        }
        return sol;
    }

    /* the Ph2 origin; obtainted from replaying Ph1 solution onto root */
    Coord ph2_origin(Coord c) const;

private:
    std::array<std::array<int,DS+2>,2>                  sofar_;      // solution buffer
    std::array<std::pair<size_t,std::array<int,DS>>,2>  rsolution_;  // reverse of solution
};

/*!
 * @brief The Kociemba's two-phase solver that searches for the optimum
 * @note The two-phase optimum is inherently not guaranteed to coincide with
 *       the theoretical optimum. 
 */
class TwoPhaseOptimSolver: public TwoPhaseSolver 
{
public:
    /*!
     * @brief Two phase solver that finds the shortest solution
     * @param c     the Coord of cube
     * @param good  early-exit threshold. This search stops as soon as a solution
     *              of length <= `good` is found. If the returned length is 
     *              > `good`, the two-phase optimum is guaranteed.
     * @return (is_solved,sol1,sol2) - solX is the PhX solution. For a legal cube
     *         coord, the `is_solved` is always true.
     */
    auto solve(const Coord &c, size_t good = 0)
        -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>;

    /* Attemp to find a solution at most `maxL` steps where the Ph1 solution to 
       `c` is of exact `d` steps; Return the legnth of solution */
    int start_search_ph1(const Coord& c, size_t d, size_t d_orig, size_t maxL);

    /* Attemp to find a Ph2 solution in exact `togo` steps */
    bool start_search_ph2(const Coord& c, size_t togo);

private:
    Coord root_;
    size_t good_;
};

} // namespace cube::solver
