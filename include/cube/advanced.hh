// SPDX-License-Identifier: MIT
// Copyright (c) 2026 coshz

#pragma once 

#include <cube/cube_export.h>

#include <filesystem>
#include <string_view>

namespace cube {

/* set directory for move / prunning tables */
CUBE_EXPORT void set_table_dir(std::filesystem::path dir);

/* get directory for move / prunning tables */
CUBE_EXPORT auto get_table_dir() -> std::filesystem::path;

/* check if tables exist in the table directory */
CUBE_EXPORT bool tables_ready();

/* load tables explicitly  */
CUBE_EXPORT void preload_tables();

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