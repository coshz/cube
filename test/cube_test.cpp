#include "cube.hh"
#include "utils.hpp"
#include <gtest/gtest.h>

TEST(CubeTest_0, BasicAssertions)
{
    auto fR = std::array<int8_t,54>{U1,U2,F3,U4,U5,F6,U7,U8,F9,R7,R4,R1,R8,R5,R2,R9,R6,R3,F1,F2,D3,F4,F5,D6,F7,F8,D9,D1,D2,B7,D4,D5,B4,D7,D8,B1,L1,L2,L3,L4,L5,L6,L7,L8,L9,U9,B2,B3,U6,B5,B6,U3,B8,B9};
    auto cR = std::string("UUFUUFUUFRRRRRRRRRFFDFFDFFDDDBDDBDDBLLLLLLLLLUBBUBBUBB");

    auto cc = CubieCube::id * std::vector<TurnAxis>{R};
    EXPECT_EQ(cc,mR);

    auto fc1 = FaceCube(cc);
    EXPECT_EQ(fc1.f.X,fR);

    auto color = fc1.color();
    EXPECT_EQ(color, cR);
    
    auto fc2 = FaceCube::fromString(color);
    EXPECT_EQ(fc1,fc2);
    EXPECT_EQ(cc,CubieCube(fc1));
}

TEST(CubeTest, BasicAssertions) 
{
    std::vector<TurnAxis> ts = {U,R,D,B,L,F,D,R};
    
    auto cc = CubieCube::id * ts;
    auto fc = FaceCube(cc);
    auto color = fc.color();

    EXPECT_EQ(cc,CubieCube(fc));
    EXPECT_EQ(fc,FaceCube(cc));
    EXPECT_EQ(fc,FaceCube::fromString(color));
}