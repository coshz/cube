#include "cube/cube.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>

using std::string;
using namespace emscripten;

val wasm_solve(
    const string &src, const string &tgt, int step, bool best
) {
    char buf[CUBE_BS] = {0};
    const char *p_src = src.empty() ? nullptr : src.c_str();
    const char *p_tgt = tgt.empty() ? nullptr : tgt.c_str();
    SolveResult sr = solve(buf, p_src, p_tgt, step, best);
    val obj = val::object();
    if(sr == SolveResultSuccess) {
        obj.set("ok", true);
        obj.set("solution", string(buf));
    } else {
        obj.set("ok", false);
        obj.set("error", string(solve_result_to_string(sr)));
    }
    return obj;
}

string wasm_facecube(
    const string &maneuver, const string &cube
) {
    char buf[CUBE_BS] = {0};
    const char *p_cube = cube.empty() ? CUBE_ID : cube.c_str();
    facecube(buf, maneuver.c_str(), p_cube);
    return string(buf);
}

string wasm_permutation(
    const string &maneuver, int fmt
) {
    char buf[CUBE_BS] = {0};
    permutation(buf, maneuver.c_str(), fmt);
    return string(buf);
}

bool wasm_solvable(
    const string &src
) {
    return solvable(src.c_str());
}

EMSCRIPTEN_BINDINGS(cube_module) {
    function("solvable", &wasm_solvable);
    function("facecube", &wasm_facecube);
    function("permutation", &wasm_permutation);
    function("solve", &wasm_solve);
}
