#pragma once 
#include <filesystem>

namespace cube::config {

/* set directory for move / prunning tables */
void set_table_dir(std::filesystem::path dir);

/* get directory for move / prunning tables */
auto get_table_dir() -> std::filesystem::path;

} // namespace cube::config
