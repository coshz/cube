#include <gtest/gtest.h>
#include "maneuver.hh"


TEST(String2ManeuverTest, BasicAssertions)
{
    auto m = "(UF'R){2} D2B"_T;
    std::vector<TurnAxis> x { U,F,F,F,R,U,F,F,F,R,D,D,B };
    EXPECT_EQ(m, x);
}

TEST(Maneuver2Cube, BasicAssertions) 
{
    auto m = Maneuver("U'R'");
    EXPECT_EQ(m.fc_color(), "UUBUUBUURFRRFRRFRRLLUFFUFFUDDLDDFDDFBBBLLLLLLDRRDBBDBB");
    EXPECT_EQ(m.cc_permutation(), "(ufl,ulb,+ubr,+dfr,-drb)(-urf)(ur,uf,ul,ub,dr,fr,br)");
}

