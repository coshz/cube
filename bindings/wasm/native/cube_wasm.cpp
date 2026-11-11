#include "cube/advanced.hh"
#include "cube/cube.hh"

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>

using namespace emscripten;

val wasm_solve(
    std::string src, std::string tgt, int good
) {
    auto sr = cube::solve(src, tgt, good);
    val obj = val::object();
    if(sr.is_success()) {
        obj.set("ok", true);
        obj.set("solution", sr.maneuver);
    } else {
        obj.set("ok", false);
        obj.set("error", std::string(to_string(sr.status)));
    }
    return obj;
}

EMSCRIPTEN_BINDINGS(cube_module) {
    constant("cubeId", std::string(cube::cubeId));
    function("cube_init",+[](bool use_symmetry, std::string table_dir){
        cube::cube_init(use_symmetry, table_dir);
    });
    function("solve", &wasm_solve);
    function("facecube", +[](std::string maneuver, std::string cube) {
        return cube::apply_maneuver(maneuver, cube);
    });
    function("permutation", +[](std::string ms_or_cube, int fmt) {
        return cube::show_permutation(ms_or_cube, static_cast<cube::PermFormat>(fmt));
    });
    function("solvable", +[](std::string src) {
        return cube::is_solvable(src);
    });
}
