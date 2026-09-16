#pragma once

#include <string_view>
#include <filesystem>

namespace cube::internal
{

/* set directory for move / prunning tables */
void set_table_dir(std::string_view dir);

/* get directory for move / prunning tables */
auto get_table_dir() -> std::filesystem::path;

/* check if tables exist in the table directory */
bool is_table_ready();

/* load tables explicitly  */
void preload_tables();

} // namespace cube::internal

