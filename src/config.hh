#pragma once
#include <filesystem>

namespace cube::config {

/* set directory for move / prunning tables */
void set_table_dir(std::filesystem::path dir);

/*!
 * @brief get directory for move / prunning tables
 *
 * Resolve order:
 * - user-settings (set via @link set_table_dir)
 * - environment variable (`CUBE_TABLE_DIR`)
 * - system cache directory
 * - working directory
 */
auto get_table_dir() -> std::filesystem::path;

/* getter of `use_symmetry` */
bool use_symmetry();

/* setter of `use_symmetry` ; only the first call takes effect */
void use_symmetry(bool v);

} // namespace cube::config
