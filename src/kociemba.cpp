#include "kociemba.hh"
#include "coord.hh"
#include "rubik.hh"
#include "data.hpp"
#include "table_ops.hh"
#include <algorithm>
#include <tuple>
#include <vector>

namespace cube::solver {

using namespace cube::pdb::ops;

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

auto KociembaSolver::solve(const CubieCube &cc, size_t good)
    -> std::tuple<bool,std::vector<TurnMove>,std::vector<TurnMove>>
{
    size_t bestL = SearchContext::DS + 1;
    auto ctx = SearchContext(cc,good);
    ctx.reset<Ph1>();
    auto c = CoordOps::origin<Ph1>(cc);
    for(auto togo = CoordOps::distance<Ph1>(c); togo < bestL; ++togo)
    {
        int l = start_search_ph1(ctx, c,togo,togo,bestL-1);
        if(l >= 0) bestL = std::min(bestL,size_t(l));
        if(bestL <= ctx.tgtlen()) break;
    }
    if(bestL > SearchContext::DS) return {false, {}, {}};
    else return { true, ctx.extract<Ph1>(), ctx.extract<Ph2>() };
}

int KociembaSolver::start_search_ph1(
    SearchContext& ctx, const Coord& c, size_t d, size_t d_orig, size_t maxL)
{
    if(d < CoordOps::distance<Ph1>(c)) return -1;
    if(d == 0) {
        ctx.commit<Ph1>(d_orig);
        ctx.reset<Ph2>();
        auto c2 = CoordOps::origin<Ph2>(ctx.root(), ctx.extract<Ph1>());
        for(auto togo = CoordOps::distance<Ph2>(c2); togo <= maxL; ++togo)
        {
            bool succ = start_search_ph2(ctx,c2,togo);
            if(succ) {
                ctx.commit<Ph2>(togo);
                return togo;
            }
        }
        return -1;
    }

    int bestL = maxL + 1;
    for(auto m: SearchContext::EM<Ph1>) 
    {
        if(is_dull_pair(m, ctx.recall<Ph1>(d))) continue;
        
        ctx.record<Ph1>(d-1,m);
        auto l = start_search_ph1(ctx, CoordOps::transform<Ph1>(c,m), d-1, d_orig, bestL-2);
        if(l >= 0 && bestL > l+1) {
            bestL = l+1;
        }
        if(bestL + d_orig - d <= ctx.tgtlen()) break; // current is good enough
        if(d >= bestL) break; // no shorter solution
    }
    return bestL > int(maxL) ? -1 : bestL;
}

bool KociembaSolver::start_search_ph2(
    SearchContext& ctx, const Coord& c, size_t togo)
{   if(togo < CoordOps::distance<Ph2>(c)) return false;
    if(togo == 0) return CoordOps::distance<Ph2>(c) == 0;
    for(auto m: SearchContext::EM<Ph2>) 
    {
        if(is_dull_pair(m, ctx.recall<Ph2>(togo))) continue;
        
        ctx.record<Ph2>(togo-1,m);
        bool ret = start_search_ph2(ctx, CoordOps::transform<Ph2>(c,m), togo-1);
        if(ret) return true;
    }
    return false;
}

} // namespace cube::solver
