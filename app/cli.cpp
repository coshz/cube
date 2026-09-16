#include "cube/cube.h"
#include "../src/internal.hh"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unordered_set>

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)

struct REPL
{
    struct S {
        std::string cmd;    /* command */
        std::string arg1;   /* solve: src | color: maneuver | perm: ms_or_cube */
        std::string arg2;   /* solve: tgt | color: cube     | perm: format */
        int         arg3;   /* solve: N */
        int         arg4;   /* solve: best */
    };

    static void run();

    static std::string help();

    static S parse_args(std::string &line);
    static void execute_cmd(const S &s);
};

std::string REPL::help()
{
    return R"(
[Help]==========================================================================
§ Commands:                                                                    §
§   solve <src> [tgt] [N] [best]                                               §
§           -- find [best] solution from <src> to [tgt] within [N] steps       §
§   color <maneuver> [cube]                                                    §
§           -- get resulting cube by applying <maneuver> to [cube]             §
§   perm  <maneuver | cube> [format]                                           §
§           -- permutate <maneuver or cube> in [format]                        §
§                0 => face representation                                      §
§                1 => cubie representation                                     §
§                2 => cycle notation                                           §
§                                                                              §
§ Arguments:                                                                   §
§   <...>               -- required positional argument                        §
§   [...]               -- optional positional argument                        §
§                                                                              §
§ Defaults:                                                                    §
§   tgt,cube            :: cid (the identity cube)                             §
§   N                   :: 30                                                  §
§   best                :: 1                                                   §
§   format              :: 2                                                   §
§                                                                              §
§ Remarks:                                                                     §
§       cube            :: X{54}                                               §
§   maneuver            :: "" | X M (" " X M)*                                 §
§          X            :: "U" | "R" | "F" | "D" | "L" | "B"                   §
§          M            :: "" | "2" | "'"                                      §
§                                                                              §
§   a cube is solvable if: cube == facecube(M,id) for some maneuver M          §
§   perm of format:                                                            §
§        0   eg. U1U2U3...B9 (len=108)                                         §
§        1   eg. ABCDEFGH00000000opqrstuvwxyz000000000000 (len=40)             §
§        2   eg. (ufl,urf,ubr)(uf,ul,ur)(+u)(−d)                               §
§                                                                              §
§ Scheme:                                                                      §
§              ^ U                                                             §
§              |                                                               §
§         C ------- r ------- D                   U1 U2 U3                     §
§        /|                  /|                   U4 U5 U6                     §
§       q |                 o |                   U7 U8 U9                     §
§      /  y                /  z          L1 L2 L3 F1 F2 F3 R1 R2 R3 B1 B2 B3   §
§     B ------- p ------- A   |  --> R   L4 L5 L6 F4 F5 F6 R4 R5 R6 B4 B5 B6   §
§     |   |               |   |          L7 L8 L9 F7 F8 F9 R7 R8 R9 B7 B8 B9   §
§     |   G . . . v . . . | . H                   D1 D2 D3                     §
§     x  .                w  /                    D4 D5 D6                     §
§     | u                 | s                     D7 D8 D9                     §
§     |.                  |/                                                   §
§     F ------- t ------- E                                                    §
§    /                                                                         §
§   v F                                                                        §
§______________________________________________________________________________§
)";
}

auto REPL::parse_args(std::string &line) -> REPL::S
{
    S obj{"","","",-0xfe,-0xfe};
    std::stringstream ss(line);
    std::string s0,s1,s2,s3,s4;
    ss >> s0 >> std::quoted(s1) >> std::quoted(s2) >> s3 >> s4;
  
    obj.cmd = s0;

    if(obj.cmd == "solve") {
        obj.arg1 = s1=="cid" || s1.empty() ? CUBE_ID : s1;
        obj.arg2 = s2=="cid" || s2.empty() ? CUBE_ID : s2;
    }
    else if(obj.cmd == "color") {
        obj.arg1 = s1;
        obj.arg2 = s2=="cid" || s2.empty() ? CUBE_ID : s2;
    }
    else if(obj.cmd == "perm") {
        obj.arg1 = s1 == "cid" || s1.empty() ? CUBE_ID : s1;
        obj.arg2 = s2;
    }

    if(s3.empty()) obj.arg3 = 30;
    else try { obj.arg3 = std::stoi(s3); } catch(...) {}
    if(s4.empty()) obj.arg4 = 1;
    else try { obj.arg4 = std::stoi(s4); } catch(...) {}
    return obj;
}

void REPL::execute_cmd(const REPL::S &s) {
    char result[CUBE_BS] = {0};
  
    if (s.cmd == "solve") {
        if (s.arg3 < 0 || s.arg4 < 0) {
            std::cout << "!!! solve: invalid arguments\n";
            return;
        }
        SolveResult sr = solve(result, s.arg1.c_str(), s.arg2.c_str(), s.arg3, s.arg4);
        if (sr != SolveResultSuccess) {
            std::cout << "!!! " << solve_result_to_string(sr) << std::endl;
            return;
        }
    } else if (s.cmd == "color") {
        bool success = facecube(result, s.arg1.c_str(), s.arg2.c_str());
        if (!success) {
            std::cout << "!!! invalid maneuver or cube (wrap maneuver in quotes if it has spaces)" << std::endl;
            return;
        }
    } else if (s.cmd == "perm") {
        int format = 2;
        try { format = std::stoi(s.arg2); } catch(...) {}
        bool success = permutation(result, s.arg1.c_str(), format);
        if (!success) {
            std::cout << "!!! invalid cube or maneuver (wrap maneuver in quotes if it has spaces)" << std::endl;
            return;
        }
    } else {
        std::cout << "!!! unsupported command `" << s.cmd << "`" << std::endl;
        return;
    }
    std::cout << result << std::endl;
}

void REPL::run()
{
    size_t no=0;
    char result[CUBE_BS];

    std::cout <<
        "Welcome to icube " STR(CUBE_VERSION_FULL) "!"
        "\n(* `:h` for help, `:q` for quit *)\n"
    ;

    while(true)
    {
        no++;
        std::string in;
        std::cout << "\nIn [" << no << "] := ";

        if(!std::getline(std::cin, in)) break;

        if(in.empty()) continue;

        const auto s = parse_args(in);

        if(s.cmd == ":q") break;
        if(s.cmd == ":h") { std::cout << help(); continue; }
      
        std::cout << "\nOut[" << no << "] => ";
        execute_cmd(s);
    }
    std::cout << "\nGoodbye!" << std::endl;
    std::cout << std::endl; // [bug: terminal reflow makes output disappear]
}

/*!
 * @note It'll take seconds to generate tables when you first run the program;
 * please be patient.
 */
int main(int argc, char *argv[])
{
    bool verbose = false;

    std::string custom_dir = "";
    std::string inline_cmd = "";

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << REPL::help();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "icube version " STR(CUBE_VERSION_FULL) << std::endl;
            return 0;
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--table-dir") {
            if (i + 1 < argc) {
                custom_dir = argv[++i];
                if(!custom_dir.empty()) cube::internal::set_table_dir(custom_dir);
            }
        } else {
            if (!inline_cmd.empty()) inline_cmd += " ";
            if (arg.find(' ') != std::string::npos) {
                inline_cmd += "\"" + arg + "\"";
            } else {
                inline_cmd += arg;
            }
        }
    }
    const bool first_run = !cube::internal::is_table_ready();
    auto table_dir = cube::internal::get_table_dir().string();

    if(first_run) {
        std::cout << "[Info] First run detected. \n"
                  << "[Info] Tables will be built under: " << table_dir << ".\n"
                  << "[Info] This may take seconds, please wait..." << std::flush;
    } else if(verbose) {
        std::cout << "[Info] Loading tables from " << table_dir << "..." << std::flush;
    }

    cube::internal::preload_tables();

    if(first_run || verbose) std::cout << " Done!\n" << std::endl;

    // inline mode
    if (!inline_cmd.empty()) {
        auto s = REPL::parse_args(inline_cmd);
        REPL::execute_cmd(s);
        return 0;
    }

    // repl mode
    REPL::run();
}