#include "cube/cube.hh"
#include "cube/advanced.hh"
#include "cube/version.h"

#include <cstddef>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std::string_literals;

#define CUBE_STRINGIFY_IMPL(x) #x
#define CUBE_STRINGIFY(x) CUBE_STRINGIFY_IMPL(x)

#define CUBE_VERSION_STRING \
    CUBE_STRINGIFY(CUBE_VERSION_FULL)

#define CUBE_WELCOME_MESSAGE \
    "Welcome to icube (" CUBE_VERSION_STRING ")!\n" \
    "(* `:h` for help, `:q` for quit *)\n"

#define CUBE_ABOUT_MESSAGE \
    "Author: coshz <fsinhx@gmail.com>\n" \
    "License: MIT\n" \
    "Homepage: https://github.com/coshz/cube\n" \
    "Build: " CUBE_VERSION_STRING "\n"

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
    static std::string help(bool full=false);
    static std::string cli_help(bool full=false);
    static S parse_args(const std::string &line);
    static std::string execute_cmd(const S &s);
};

std::string REPL::cli_help(bool full)
{
    std::string usage = 
R"(
[Usage]
  icube                     Start interactive REPL
  icube <cmd> [args...]     Run one command and exit
  icube [options]

[Options]
  -h, --help                Show this help plus short help for commands
  -H, --help-full           Show this help plus full help for commands
  -v, --version             Show version
  -a, --about               Show about info
      --verbose             Print progress messages
      --table-dir <dir>     Use a custom table directory
)";
    return usage + help(full);
}

std::string REPL::help(bool full)
{
    return !full ? 
R"(
[Help]==========================================================================
§ Commands:                                                                    §
§   solve <src> [tgt] [N] [best]    Find solution (tgt=cid, N=30, best=1)      §
§   color <maneuver> [cube]         Apply maneuver (cube=cid)                  §
§   perm  <maneuver|cube> [format]  Display permutation (format=2)             §
§   :h / :hh / :q                   Help / Full Help / Quit                    §
§ Defaults: tgt/cube=cid, N=30, best=1, format=2.                              §
§ Cube: 54-char string. Maneuver: e.g. "R U R' U'".                            §
§ Format: 0 => 54-face; 1 => 20-cubie; 2 => cycles.                            §
§______________________________________________________________________________§
)"
:
R"(
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
§   perm format:                                                               §
§        0   eg. U1U2U3...B9 (len=108)                                         §
§        1   eg. ABCDEFGH00000000opqrstuvwxyz000000000000 (len=40)             §
§        2   eg. (ufl,urf,ubr)(uf,ul,ur)(+u)(−d)                               §
§                                                                              §
§ Notations of the Rubik's Cube:                                               §
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

auto REPL::parse_args(const std::string &line) -> REPL::S
{
    S obj{"","","",-0xfe,-0xfe};
    std::stringstream ss(line);
    std::string s0,s1,s2,s3,s4;
    ss >> s0 >> std::quoted(s1) >> std::quoted(s2) >> s3 >> s4;
    obj.cmd = s0;

    auto resolve_cube = [](const std::string& arg) -> std::string {
        return arg =="cid" || arg.empty() ? std::string(cube::cubeId) : arg;
    };

    if(obj.cmd == "solve") {
        obj.arg1 = resolve_cube(s1);
        obj.arg2 = resolve_cube(s2);
    }
    else if(obj.cmd == "color") {
        obj.arg1 = s1;
        obj.arg2 = resolve_cube(s2);
    }
    else if(obj.cmd == "perm") {
        obj.arg1 = resolve_cube(s1);
        obj.arg2 = s2;
    }

    if(s3.empty()) obj.arg3 = 30;
    else try { obj.arg3 = std::stoi(s3); } catch(...) {}
    if(s4.empty()) obj.arg4 = 1;
    else try { obj.arg4 = std::stoi(s4); } catch(...) {}
    return obj;
}

auto REPL::execute_cmd(const REPL::S &s) -> std::string {
    if (s.cmd == "solve") {
        if (s.arg3 < 0 || s.arg4 < 0) {
            return "!!! solve: invalid arguments";
        }
        auto sol = cube::solve(s.arg1, s.arg2, s.arg3, s.arg4);
        return sol.is_success() 
            ? sol.maneuver
            : "!!! "s += cube::to_string(sol.status);
    } else if (s.cmd == "color") {
        try {
            return cube::apply_maneuver(s.arg1, s.arg2);
        } catch(...) {
            return "!!! invalid maneuver or cube";
        }
    } else if (s.cmd == "perm") {
        int fmt_val = 2;
        if(!s.arg2.empty()) {
            try { 
                fmt_val = std::stoi(s.arg2); 
            } catch(...) {
                return "!!! perm: invalid format";
            } 
            if (fmt_val < 0 || fmt_val > 2) 
                return "!!! perm: format must be 0, 1 or 2";
        }
        try {
            return cube::show_permutation(s.arg1, static_cast<cube::PermFormat>(fmt_val));
        } catch(...) {
            return "!!! invalid cube or maneuver";
        }
    } else {
        return "!!! unsupported command `"s + s.cmd + "`";
    }
}

void REPL::run()
{
    std::cout << CUBE_WELCOME_MESSAGE;
    for(std::size_t no = 1;;no++)
    {
        std::string in;
        std::cout << "\nIn [" << no << "] := ";

        if(!std::getline(std::cin, in)) break;
        if(in.empty()) continue;

        const auto s = parse_args(in);
        if(s.cmd == ":q")   break;
        if(s.cmd == ":h")   { std::cout << help(false); continue; }
        if(s.cmd == ":hh")  { std::cout << help(true); continue; }
        std::cout << "\nOut[" << no << "] => " << execute_cmd(s) << std::endl;
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
            std::cout << REPL::cli_help(false);
            return 0;
        } else if (arg == "-H" || arg == "--help-full") {
            std::cout << REPL::cli_help(true);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout <<  CUBE_VERSION_STRING << std::endl;
            return 0;
        } else if (arg == "-a" || arg == "--about") {
            std::cout <<  CUBE_ABOUT_MESSAGE;
            return 0;
        } if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--table-dir") {
            if (i + 1 < argc) { custom_dir = argv[++i]; }
        } else {
            if (!inline_cmd.empty()) inline_cmd += " ";
            if (arg.find(' ') != std::string::npos) {
                inline_cmd += "\"" + arg + "\"";
            } else {
                inline_cmd += arg;
            }
        }
    }
    if(!custom_dir.empty()) cube::set_table_dir(custom_dir);
    if(verbose) {
        std::cout << "[Info] Using table directory: " << cube::get_table_dir().string() << std::endl;
        if(!cube::tables_ready()) {
            std::cout << "[Info] Tables not found, building..." << std::flush;
        } else {
            std::cout << "[Info] Tables found, loading..." << std::flush;
        }
    }
    cube::preload_tables();
    if(verbose) std::cout << " Done!" << std::endl;
    if(!inline_cmd.empty()) {
        if(verbose) std::cout << "[Info] Running inline command: " << inline_cmd << std::endl;
        auto s = REPL::parse_args(inline_cmd);
        std::cout << REPL::execute_cmd(s) << std::endl;
    } else {
        if(verbose) std::cout << "[Info] Entering REPL mode..." << std::endl;
        REPL::run();
    }
}