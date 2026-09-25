#include "config.hh"
#include "utils.hpp"
#include <cstdlib>
#include <filesystem>

namespace cube::config 
{
namespace fs = std::filesystem;

static fs::path& get_custom_dir_storage()
{
    static fs::path custom_dir = "";
    return custom_dir;
}

void set_table_dir(fs::path dir)
{
    get_custom_dir_storage() = dir;
}

fs::path get_table_dir()
{
    // user-settings
    const auto user_dir = get_custom_dir_storage();
    if(!user_dir.empty()) return user_dir;

    // environment variable
    if(const char *env = std::getenv("CUBE_TABLE_DIR")) {
        if(env[0] != '\0') return fs::path(env);
    }
    
    try {
        // system cache dir
        return utils::get_cache_dir() / "cube.coshz" / "tables";
    } catch (...) {
        return fs::absolute("tables");
    }
}

} // namespace cube::config