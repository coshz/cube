#include "rubik.hh"
#include "show.hpp"
#include "utils.hpp"

#include <gtest/gtest.h>

using namespace cube;

TEST(CubeTest_0, BasicAssertions)
{
    auto fR = std::array<int8_t,54>{U1,U2,F3,U4,U5,F6,U7,U8,F9,R7,R4,R1,R8,R5,R2,R9,R6,R3,F1,F2,D3,F4,F5,D6,F7,F8,D9,D1,D2,B7,D4,D5,B4,D7,D8,B1,L1,L2,L3,L4,L5,L6,L7,L8,L9,U9,B2,B3,U6,B5,B6,U3,B8,B9};
    auto cR = std::string("UUFUUFUUFRRRRRRRRRFFDFFDFFDDDBDDBDDBLLLLLLLLLUBBUBBUBB");

    auto cc = CubieCube::id * std::vector<TurnMove>{Rx1};
    EXPECT_EQ(cc,mR);

    auto fc1 = cc.toFaceCube();
    EXPECT_EQ(fc1.f.X,fR);
    EXPECT_EQ(fc1.toCubieCube(),cc);
    EXPECT_EQ(cR,show::to_string(ColorState::fromString(cR)));
}

TEST(CubeTest, BasicAssertions) 
{
    std::vector<TurnMove> ts = {Ux1,Rx1,Dx2,Bx3,Lx2,Fx1,Dx3,Rx2};
    auto fc = FaceCube::id * ts;
    auto cc = CubieCube::id * ts;
    EXPECT_EQ(cc, fc.toCubieCube());
    EXPECT_EQ(fc, cc.toFaceCube());
}
