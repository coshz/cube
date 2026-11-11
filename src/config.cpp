#include "config.hh"
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <stdexcept>

namespace cube::config
{
namespace fs = std::filesystem;

namespace {
bool& use_symmetry_ref()
{
    static bool val_ = CUBE_USE_SYMMETRY;
    return val_;
}

fs::path& get_custom_dir_storage()
{
    static fs::path custom_dir = {};
    return custom_dir;
}

/* get the system cache directory */
auto get_cache_dir() -> std::filesystem::path
{
#ifdef _WIN32
    const char* localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData) return std::filesystem::path(localAppData);
#elif __APPLE__
    const char* home = std::getenv("HOME");
    if (home) return std::filesystem::path(home) / "Library" / "Caches";
#else
    const char* xdgCache = std::getenv("XDG_CACHE_HOME");
    if (xdgCache) return std::filesystem::path(xdgCache);
    const char* home = std::getenv("HOME");
    if (home) return std::filesystem::path(home) / ".cache";
#endif
    throw std::runtime_error("Unable to determine cache path");
}
}

bool use_symmetry() { return use_symmetry_ref(); }
void use_symmetry(bool v)
{
    static std::once_flag once;
    std::call_once(once, [v]() { use_symmetry_ref() = v; });
}


void set_table_dir(fs::path dir)
{
    static std::once_flag once;
    std::call_once(once, [dir]() { get_custom_dir_storage() = dir; });
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
        return get_cache_dir() / "cube.coshz" / "tables";
    } catch (...) {
        return fs::absolute("tables");
    }
}

} // namespace cube::config
