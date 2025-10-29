#include <gtest/gtest.h>
#include "libcube.h"
#include "cube.hh"

#define BS  100
#define cid "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
    
bool check_solution(const char *cube, const char *solution)
{
    std::string s;
    char buffer[BS];
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
    facecube(s.c_str(), buffer);
    return strcmp(cube, buffer) == 0;
}

TEST(LibCubeTest, BasicAssertions)
{
    char cube[60], buffer[100];
    // const char *m = "(U2FDL'RU){5} D'LDR";
    const char *m = "URF";
    facecube(m, cube);
    EXPECT_STREQ(cube, "UURUUFLLFURBURBFRBFFRFFRDDDRRRDDBDDLFFDLLDLLBULLUBBUBB");
    int rc = solve(cube, NULL, buffer, 30);
    EXPECT_EQ(rc,0);
    EXPECT_TRUE(check_solution(cube, buffer));
}