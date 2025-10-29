#include "cube/libcube.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unordered_set>

#define _VERSION_   "0.2.0"
#define _AUTHOR_    "Coshz"

struct REPL
{
    struct S { std::string cmd,arg1,arg2; int arg3,arg4; };
    static S parse_args(std::string &line);

    static std::string help();
    static void run();
};

auto REPL::parse_args(std::string &line) -> REPL::S
{
    S obj{"","","",-0xfe,-0xfe};
    std::stringstream ss(line);
    std::string s0,s1,s2,s3,s4;
    ss >> s0 >> std::quoted(s1) >> std::quoted(s2) >> s3 >> s4;
    
    obj.cmd = s0;
    if(obj.cmd == "solve") {
        obj.arg1 = s1=="cid" || s1 == "" ? CUBE_ID : s1;
        obj.arg2 = s2=="cid" || s2 == "" ? CUBE_ID : s2;
    } else {
        obj.arg1 = s1;
        obj.arg2 = s2=="cid" || s2 == "" ? CUBE_ID : s2;
    }
    
    if(s3.empty()) obj.arg3 = 30;
    else try { obj.arg3 = std::stoi(s3); } catch(...) {}
    if(s4.empty()) obj.arg4 = 1; 
    else try { obj.arg4 = std::stoi(s4); } catch(...) {}
    return obj;
}

std::string REPL::help()
{
    return 
    "[Help]\n"
    "    solve <src> [tgt=cid] [best=1] [N=30]  -- find [best] solution form <src> to [tgt] within [N] steps\n"
    "    color <maneuver> [cube=cid]            -- color by applying maneuver to cube\n"
    "    perm  <maneuver>                       -- decompose maneuver to cubies permutation\n"
    "\nwhere:\n"
    "   <...>           -- required argument\n"
    "   [...]           -- optional argument\n"
    "   src,tgt,cube    :: the color configuration; eg: `UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`\n"
    "   maneuver        :: the move sequence;       eg: `FRL'B2D`, `(DR'F2L){7} BD2`\n"
    ;
}

void REPL::run()
{
    size_t no=0;
    char result[CUBE_BS];
    int rc;

    std::cout << "Welcome! This is a Rubik's cube solver.\n"
                 "(*`:h` for help, `:q` for quit *)\n";

    while(true)
    {
        std::string in;
        std::cout << "\nIn [" << no << "] := ";
        std::getline(std::cin, in);

        if(in == "") continue;

        const auto s = parse_args(in);

        if(s.cmd == ":q") break;
        if(s.cmd == ":h") { std::cout << help(); continue; }
        
        std::cout << "\nOut[" << no << "] => ";
    
        if(s.cmd == "solve") {
            if(s.arg3<0 || s.arg4 <0) { std::cout << "!!! solve: invalid arguments\n"; continue; }
            rc = solve_ultimate(s.arg1.c_str(), s.arg2.c_str(), result, s.arg3, s.arg4, 1);
            switch(rc) 
            {
                case CODE_INVALID_SRC:  std::cout << "!!! the source cube is invalid\n"; break;
                case CODE_INVALID_TGT:  std::cout << "!!! the target cube is invalid\n"; break;
                case CODE_UNSOLVABLE:   std::cout << "!!! unsolvable\n"; break;
                case CODE_NOT_FOUND:    std::cout << "!!! solution not found since N is too small\n"; break;
                default:;// CODE_OK
            }
        } else if(s.cmd == "color") {
            facecube(s.arg2.c_str(), s.arg1.c_str(), result);
        } else if(s.cmd == "perm") {
            permutation(s.arg1.c_str(), result);  
        }else {
            std::cout << "!!! unsupported command `" << s.cmd << "`\n";
            continue;
        }
        std::cout << result << '\n';
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
    REPL::run();
}