#include "internal.hh"
#include "table.hh"
#include "utils.hpp"

namespace cube::internal {

namespace fs = std::filesystem;

static std::string& get_custom_dir_storage()
{
    static std::string custom_dir = "";
    return custom_dir;
}

void set_table_dir(std::string_view dir)
{
    get_custom_dir_storage() = dir;
}

fs::path get_table_dir()
{
    // user-settings
    std::string user_dir = get_custom_dir_storage();
    if(!user_dir.empty()) return fs::path(user_dir);

    // environment variable
    if(const char *env = std::getenv("CUBE_TABLE_DIR")) {
        if(env[0] != '\0') return fs::path(env);
    }

    // system cache dir
    try {
        return cube::utils::get_cache_dir() / "cube" / "tables";
    } catch (...) {}

    // fallback: current dir
    return fs::current_path() / "tables";
}

bool is_table_ready()
{
    return fs::exists(get_table_dir() / "tm_twist.dat");
}

void preload_tables()
{
    (void)pdb::get_TM();
    (void)pdb::get_TP();
}

} // namespace cube::internal