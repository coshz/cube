#include <string>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include "libcube.h"

#define _VERSION_   "0.1.1"
#define _AUTHOR_    "Coshz"


struct REPL
{
    static void run();
    static const char* help();
};

const char* REPL::help()
{
    return 
    "[Help]\n"
    "    solve      <cube>  [tgt]       -- `solve2 30 <cube> [tgt]`\n"
    "    solve2  N  <cube>  [tgt]       -- solve src cube to tgt(default:cid) in N steps if possible\n"
    "    info L '<maneuver>'            -- resolve [maneuver] to L (1:color configuration; 2:cubie permutation)\n"
    "\nwhere:\n"
    "   <cube> is the color configuration of cube;  eg: `UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`\n"
    "   <maneuver> is the move sequence;            eg: `FRL'B2D`, `(DR'F2L){7} BD2`\n"
    ;
}

void REPL::run()
{
    size_t no=0;
    const size_t BS = 100;
    char result[BS];
    int rc;

    std::cout << "Welcome! This is a Rubik's cube solver.\n"
                 "(*`:h` for help, `:q` for quit *)\n";

    while(true)
    {
        std::string in;
        std::cout << "\nIn [" << no << "] := ";
        std::getline(std::cin, in);

        if(in == "") continue;

        std::string cmd, arg1, arg2, arg3;
        std::stringstream ss(in);

        ss >> cmd;
        if(cmd == ":q") break;
        if (cmd == ":h") { std::cout << help(); continue; }
        
        std::cout << "\nOut[" << no << "] => ";
    
        if (cmd == "info") {
            auto strip = [](const std::string &s, const std::unordered_set<char> &cs) {
                auto it1 = std::find_if_not(s.begin(), s.end(), 
                    [&cs](auto &c) { return cs.count(c) > 0; }
                );
                auto it2 = std::find_if_not(s.rbegin(), s.rend(), 
                    [&cs](auto &c) { return cs.count(c) > 0; }
                ).base();
                return it1 <= it2 ? std::string(it1,it2) : std::string();
            };

            ss >> arg1; std::getline(ss,arg2);
            std::unordered_set chs = {'"','\'',' '};

            if(arg1 == "1") facecube(strip(arg2,chs).c_str(), result);     
            else if(arg1 == "2") permutation(strip(arg2,chs).c_str(), result);  
            else {
                std::cout << "!!! unsupported argument `" << arg1 << "`\n";
                continue;
            }
            std::cout << result << '\n';
        } 
        else if (cmd == "solve" || cmd == "solve2") {
            ss >> arg1 >> arg2 >> arg3;
            if(cmd == "solve")
            rc = solve(arg1 == "" ? NULL : arg1.c_str(), 
                        arg2 == "" ? NULL : arg2.c_str(),
                        result, 30);
            else
            rc = solve(arg2 == "" ? NULL : arg2.c_str(), 
                        arg3 == "" ? NULL : arg3.c_str(),
                        result, std::stoi(arg1));

            switch(rc) {
            case -1: std::cout << "!!! the source cube is invalid\n"; break;
            case -2: std::cout << "!!! the target cube is invalid\n"; break;
            case 1:  std::cout << "!!! unsolvable\n"; break;
            case 2:  std::cout << "!!! solution not found since N is too small\n"; break;
            default: // 0
                std::string solution{};
                solution += "[";
                for(char *p = result; *p != '\0'; p++) {
                    solution = solution + " " + Move2Str[*p - 1];
                }
                solution += (solution.length() == 1) ? "]": " ]";
                std::cout << solution << '\n';
            }
        }
        else {
            std::cout << "!!! unsupported command `" << cmd << "`\n";
            continue;
        }
        no++;
    }
    std::cout << "\nGoodbye!\n";
}

/*!
 * @note It'll take seconds to generate tables when you first run the program;
 * please be patient. 
 */
int main(int argc, char *argv[])
{
    auto repl = REPL();
    repl.run();
}