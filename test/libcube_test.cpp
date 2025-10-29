#include "cube/libcube.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <string>
#include <cstring>

const std::string Move2Str[18] = { "U","U2","U'","R","R2","R'","F","F2","F'","D","D2","D'","L","L2","L'","B","B2","B'" };

bool check_solution(const char *cube, const char *solution)
{
    std::string s;
    char buffer[CUBE_BS];
    auto inv = [](int i) -> int {
        switch(i % 3) {
            case 0: return i + 2;
            case 1: return i;
            case 2: return  i - 2;
            default: return -1; // reassure stupid compiler
        }
    };
    int n = strlen(solution);
    for(int j = n-1; j >= 0; j--) {
        s += Move2Str[inv(solution[j]-1)];
    }
    facecube(NULL, s.c_str(), buffer);
    return strcmp(cube, buffer) == 0;
}

TEST(SolveTest, BasicAssertions)
{
    char cube[CUBE_BS], buffer[CUBE_BS];
    const char *m = "URF";
    facecube(NULL, m, cube);
    EXPECT_STREQ(cube, "UURUUFLLFURBURBFRBFFRFFRDDDRRRDDBDDLFFDLLDLLBULLUBBUBB");
    int rc = solve_ultimate(cube, CUBE_ID, buffer, 30, 0, 0);
    EXPECT_EQ(rc,0);
    EXPECT_TRUE(check_solution(cube, buffer));
}

TEST(PermutationTest,BasicAssertions)
{
    char buf[CUBE_BS];

    auto m0 = "R";
    permutation(m0, buf);
    EXPECT_STREQ(buf,"(-dfr,+drb,-ubr,+urf)(fr,dr,br,ur)");
    
    // edge flip
    auto m1 = "FUD'L2U2D2RUR'D2U2L2DU'F'U'";
    permutation(m1, buf);
    EXPECT_STREQ(buf,"(+ur)(+uf)");

    // corner 3-cycle
    auto m2 = "RB'RF2R'BRF2R2"; 
    permutation(m2,buf);
    EXPECT_STREQ(buf,"(ufl,ubr,urf)");

    // edge 3-cycle
    auto m3 = "FU2L2D2BD2L2U2"; // rf br uf
    permutation(m3,buf);
    EXPECT_STREQ(buf,"(+br,fr,+uf)");
}

TEST(ColorTest,BasicAssertions)
{
    char buf[CUBE_BS],buf2[CUBE_BS];
    const char* m = "R";
    facecube(NULL, m, buf);
    EXPECT_STREQ(buf,"UUFUUFUUFRRRRRRRRRFFDFFDFFDDDBDDBDDBLLLLLLLLLUBBUBBUBB");
    facecube(buf,"F",buf2);
    EXPECT_STREQ(buf2,"UUFUUFLLLURRURRFRRFFFFFFDDDRRRDDBDDBLLDLLDLLBUBBUBBUBB");
}
