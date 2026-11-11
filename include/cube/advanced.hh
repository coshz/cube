// SPDX-License-Identifier: MIT
// Copyright (c) 2026 coshz

#pragma once

#include "cube/cube_export.h"
#include <filesystem>
#include <string_view>

#ifndef CUBE_USE_SYMMETRY
#   define CUBE_USE_SYMMETRY 1
#endif

namespace cube {

/*!
 * @brief Initialize the cube library by loading or building lookup tables.
 * @param use_symmetry Select the table set:
 *      - `false`: smaller (<10 MB), fast to build, slower solving;
 *      - `true`:  symmetry-based (~260 MB), slower to build, faster solving.
 *      This is a memory/disk/build-time vs solve-speed trade-off.
 * @param dir Directory for reading and writting table files.
 *      Empty means the default directory (see @ref get_table_dir()).
 *      Tables are loaded if present; otherwise they are built and written here.
 * @note Only the first call takes effect; call it before the first @ref solve,
 *      which invokes it with default arguments.
 * @warning With `use_symmetry = true`, the first run may take minutes and write
 *      ~260 MB of data to @p dir.
 */
CUBE_EXPORT void cube_init(
    bool use_symmetry = CUBE_USE_SYMMETRY,
    std::string_view dir = {}
);

/* set directory for move / prunning tables */
CUBE_EXPORT void set_table_dir(std::filesystem::path dir);

/* get directory for move / prunning tables */
CUBE_EXPORT auto get_table_dir() -> std::filesystem::path;

/* check if tables exist in the table directory */
// CUBE_EXPORT bool tables_ready();

/*!
 * @brief check the validity of color configuration of cube
 * @def A configuration is valid ::= it's legal (solvable) modulo edge flips or corner twists
 */
CUBE_EXPORT bool is_valid_config(std::string_view cube);

/*!
 * @brief check the validity of maneuver (moves sequence)
 * @def A maneuver is valid ::= it's made up of moves sequence
 *   {U,U2,U',R,R2,R',F,F2,F',D,D2,D',L,L2,L',B,B2,B'} joined by spaces
 */
CUBE_EXPORT bool is_valid_maneuver(std::string_view maneuver);

} // namespace cube
