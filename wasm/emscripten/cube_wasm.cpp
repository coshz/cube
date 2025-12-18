#include "cube/libcube.h"

#include <emscripten/bind.h>
#include <string>

using namespace emscripten;

auto c_solve_ultimate(const std::string &src, const std::string &tgt, int step, bool best) -> std::pair<status_code, std::string> {
    char buf[CUBE_BS]="\0";
    int rc = solve_ultimate(src.c_str(), tgt.c_str(), buf, step, best, 1);
    return std::make_pair(static_cast<status_code>(rc), std::string(buf));
}

auto c_solve(const std::string &src, bool best) -> std::pair<status_code, std::string> {
    return c_solve_ultimate(src, CUBE_ID, 30, best);
}

std::string c_facecube(const std::string &maneuver, const std::string &cube) {
    char buf[CUBE_BS];
    facecube(cube.c_str(), maneuver.c_str(), buf);
    return std::string(buf);
}

bool c_solvable(const std::string &src) {
    return solvable(src.c_str());
}

std::string c_permutation(const std::string &maneuver) {
    char buf[CUBE_BS];
    permutation(maneuver.c_str(), buf);
    return std::string(buf);
}

EMSCRIPTEN_BINDINGS(cube_module) {
    function("js_solve", &c_solve);
    function("js_solve_ultimate", &c_solve_ultimate);
    function("js_facecube", &c_facecube);
    function("js_permutation", &c_permutation);
    function("js_solvable", &c_solvable);
    enum_<status_code>("status_code") 
        .value("Ok", CODE_OK)
        .value("Unsolvable", CODE_UNSOLVABLE)
        .value("Not_found", CODE_NOT_FOUND)
        .value("Bad_src", CODE_INVALID_SRC)
        .value("Bad_tgt", CODE_INVALID_TGT)
		.value("Unknown_err",CODE_UNKNOWN_ERROR)
    ;
    value_array<std::pair<status_code,std::string>>("solve_result_t")
        .element(&std::pair<status_code, std::string>::first)
        .element(&std::pair<status_code, std::string>::second)
    ;
}
