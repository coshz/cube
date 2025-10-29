#pragma once
#include <array>
#include <vector>
#include <tuple>
#include "def.h"
#include "coord.hh"
#include "cube.hh"
#include "table.hh"

/*! 
 * @brief Kociemba's twophase algorithm
 */
class TwoPhaseSolver 
{
public:
    // auto solveCube(const CubieCube &cc, size_t max_step=30) 
    // -> std::tuple<bool, std::vector<TurnMove>, std::vector<TurnMove>>;
    
    // find the first solution of length < max_step
    auto solve(const Coord &c, size_t max_step=30) 
    -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>;

protected:
    enum enum_phase { Ph1=0, Ph2=1 };
    
    void search_phase1(const Coord &c, size_t togo);
    void search_phase2(const Coord &c, size_t togo);

    /* extract solution of phase 1/2 from sofar_ph */
    template<enum_phase I> std::vector<TurnMove> get_solution_ph() const;

    /* move-table based coord transform */
    template<enum_phase I> static Coord transform(const Coord &c, const TurnMove &m);

    /* prunning-talbe based lower bound distance in phase 1/2 */
    template<enum_phase I> static size_t distance(const Coord &c);

    /* subgroup G1,G2 indicating the end of phase 1/2 */
    template<enum_phase I> static bool isInSubG(const Coord &c);

    /* max search depth for phase 1/2 */
    static constexpr size_t depth_ph1 = 20, depth_ph2 = 15;

    /* elementary moves of two phases */
    static constexpr std::array<TurnMove,18> 
    EM0 = { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
    static constexpr std::array<TurnMove,10> 
    EM1 = { Ux1,Ux2,Ux3,Rx2,Fx2,Dx1,Dx2,Dx3,Lx2,Bx2 };

private:
    /* state data shared for search phases */

    Coord cube_;
    size_t max_len, cur_len;
    std::array<bool,2> exit_;
    std::array<std::array<int,std::max(depth_ph1,depth_ph2)>,2> sofar_;
    std::array<std::vector<TurnMove>,2> solution_;
};