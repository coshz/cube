#pragma once

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <chrono>
#include <optional>
#include <stdexcept>
#include <utility>
#include <type_traits>

namespace cube::utils {

/* get the system cache directory */
inline auto get_cache_dir() -> std::filesystem::path
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

/* measure the execution time, for benchmark */
template<typename F, typename... Args>
inline auto time_execution(F&& f, Args&&... args)
{
    using Rf = std::invoke_result_t<F, Args...>;

    auto start = std::chrono::high_resolution_clock::now();
    auto duration_from_start = [start](){
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    };

    if constexpr (std::is_void_v<Rf>) {
        std::forward<F>(f)(std::forward<Args>(args)...);
        return std::make_pair(
            duration_from_start(),
            std::nullopt
        );
    } else {
        auto result = std::forward<F>(f)(std::forward<Args>(args)...);
        return std::make_pair(
            duration_from_start(),
            std::make_optional(result)
        );
    }
}
} // namespace cube::utils