#pragma once
#include "coord.hh"
#include "data.hpp"
#include "rubik.hh"
#include "table_ops.hh"
#include <algorithm>
#include <array>
#include <utility>
#include <vector>
#include <tuple>

namespace cube::solver {

using namespace cube::data;
using namespace cube::pdb;
using namespace cube::pdb::table;
using cube::pdb::ops::Phase;

class SearchContext
{
public:
    SearchContext(const CubieCube& cc = CubieCube::id, size_t good = 22)
        : root_(cc), tgtlen_(good)
    {}

    CubieCube   root() const    { return root_; }
    size_t      tgtlen() const  { return tgtlen_; }

    /* reset solution buffer */
    template<Phase PhX>
    void reset() { path_[PhX].fill(-1); }

    /* cache / fetch the move of given step */
    template<Phase PhX> void record(int step, int move){ path_[PhX][step] = move; }
    template<Phase PhX> int  recall(int step) const    { return path_[PhX][step]; }

    /* flush solution buffer to rsolution ( 0 <= L <= DS ) */
    template<Phase PhX>
    void commit(size_t L)
    {
        rsolution_[PhX].first = L;
        std::copy(path_[PhX].begin(), path_[PhX].begin()+L, rsolution_[PhX].second.begin());
    }

    /* get solution of phase 1/2 by reversing rsolution */
    template<Phase PhX>
    auto extract() const -> std::vector<TurnMove>
    {
        size_t n = rsolution_[PhX].first;
        if(n == 0) return {};
        std::vector<TurnMove> sol(n);
        for(auto i = 0; i < n; i++) {
            sol[i] = static_cast<TurnMove>(rsolution_[PhX].second[n-1-i]);
        }
        return sol;
    }

    /* max search depth for phase 1/2 (conclusion from literatures) */
    static constexpr int D0 = 12, D1 = 18, DS = D0+D1;
    template<Phase PhX> static constexpr auto D = std::get<PhX>(std::tie(D0,D1));

    /* elementary moves of two phases */
    static constexpr std::array<TurnMove,18> EM0
        = { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
    static constexpr std::array<TurnMove,10> EM1
        = { Ux1,Ux2,Ux3,Rx2,Fx2,Dx1,Dx2,Dx3,Lx2,Bx2 };
    template<Phase PhX>
    static constexpr auto& EM = std::get<PhX>(std::tie(EM0,EM1));

private:
    CubieCube root_;
    size_t tgtlen_;
    std::array<std::array<int,DS+2>,2>                  path_;      // solution buffer
    std::array<std::pair<size_t,std::array<int,DS>>,2>  rsolution_; // reverse of solution
};

/*!
 * @brief The Kociemba's algorithm to search the first solution.
 */
class KociembaSolver
{
public:
    /*!
     * @brief Two phase solver that finds the shortest solution
     * @param cc    the cube
     * @param good  early-exit threshold. This search stops as soon as a solution
     *              of length <= `good` is found. If the returned length is
     *              > `good`, the two-phase optimum is guaranteed.
     * @return (is_solved,sol1,sol2) - solX is the PhX solution. For a legal cube
     *         coord, the `is_solved` is always true.
     */
    static auto solve(const CubieCube &cc, size_t good)
        -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>;
protected:
    /* Attemp to find a solution at most `maxL` steps where the Ph1 solution to
    `c` is of exact `d` steps; Return the legnth of solution */
    static int start_search_ph1(SearchContext& ctx, const Coord& c, size_t d, size_t d_orig, size_t maxL);

    /* Attemp to find a Ph2 solution in exact `togo` steps */
    static bool start_search_ph2(SearchContext& ctx, const Coord& c, size_t togo);
};

} // namespace cube::solver
