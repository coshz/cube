#include "cube/cube.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <string>
#include <cstring>

const std::string Move2Str[18] = { "U","U2","U'","R","R2","R'","F","F2","F'","D","D2","D'","L","L2","L'","B","B2","B'" };

bool check_solution(const char *cube, const char *solution)
{
    char buffer[CUBE_BS];
    facecube(buffer, solution, cube);
    return strcmp(buffer, CUBE_ID) == 0;
}

TEST(SolveTest, BasicAssertions)
{
    char cube[CUBE_BS], buffer[CUBE_BS];
    const char *m = "URF";
    facecube(cube,m);
    EXPECT_STREQ(cube, "UURUUFLLFURBURBFRBFFRFFRDDDRRRDDBDDLFFDLLDLLBULLUBBUBB");
    int rc = solve(buffer, cube, CUBE_ID, 30, 0);
    EXPECT_EQ(rc,0);
    EXPECT_TRUE(check_solution(cube, buffer));
}

TEST(PermutationTest,BasicAssertions)
{
    char buf[CUBE_BS];

    auto m0 = "R";
    permutation(buf,m0);
    EXPECT_STREQ(buf,"(-dfr,+drb,-ubr,+urf)(fr,dr,br,ur)");
    
    // edge flip
    auto m1 = "FUD'L2U2D2RUR'D2U2L2DU'F'U'";
    permutation(buf,m1);
    EXPECT_STREQ(buf,"(+ur)(+uf)");

    // corner 3-cycle
    auto m2 = "RB'RF2R'BRF2R2"; 
    permutation(buf,m2);
    EXPECT_STREQ(buf,"(ufl,ubr,urf)");

    // edge 3-cycle
    auto m3 = "FU2L2D2BD2L2U2"; // rf br uf
    permutation(buf,m3);
    EXPECT_STREQ(buf,"(+br,fr,+uf)");
}

TEST(ColorTest,BasicAssertions)
{
    char buf[CUBE_BS],buf2[CUBE_BS];
    const char* m = "R";
    facecube(buf,m);
    EXPECT_STREQ(buf,"UUFUUFUUFRRRRRRRRRFFDFFDFFDDDBDDBDDBLLLLLLLLLUBBUBBUBB");
    facecube(buf2,"F",buf);
    EXPECT_STREQ(buf2,"UUFUUFLLLURRURRFRRFFFFFFDDDRRRDDBDDBLLDLLDLLBUBBUBBUBB");
}
