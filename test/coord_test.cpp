#include <gtest/gtest.h>
#include "cube.hh"
#include "coord.hh"
#include "maneuver.hh"


TEST(MoveTest, BasicAssertion)
{
    // Move transform: maneuver vs CubieCube
    std::vector<TurnAxis> ms { F,U,D,F,R,U,F,R,L,U,B,R };
    CubieCube cc = CubieCube::id;
    for(auto &m: ms) cc = cc * ElementaryMove[3*m];
    std::string color1 = FaceCube(cc).color();

    std::string color2 = Maneuver(ms).fc_color();

    EXPECT_EQ(color1, color2);
}

