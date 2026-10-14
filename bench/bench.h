#pragma once

#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <ctime>
#include <array>
#include <vector>
#include <chrono>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <utility>
#include <type_traits>
#include <iostream>
#include <iomanip>
#include <sstream>

/* Measure the execution time, for benchmark */
template<typename F, typename... Args>
auto time_execution(F&& f, Args&&... args)
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

/* Generate a random maneuver of length `length` */
inline auto generate_random_maneuver(const size_t length, std::mt19937& rng) -> std::string
{
    if(length == 0) return {};

    auto is_dull_pair = [](const int A, const int B) {
        return (A/3 == B/3) || (A/3 == B/3+3);
    };

    static constexpr std::array<std::string_view,18> Move2Str = { 
        "U","U2","U'","R","R2","R'","F","F2","F'","D","D2","D'","L","L2","L'","B","B2","B'" 
    };

    std::uniform_int_distribution<int> move_dist(0,17);
    std::string maneuver;
    maneuver.reserve(length*3);
    std::vector<int> moves;
    moves.reserve(length);

    int move = move_dist(rng);
    moves.push_back(move);
    maneuver += Move2Str[move];
    while(moves.size() < length) 
    {
        move = move_dist(rng);
        if(is_dull_pair(move, moves.back())) continue;
        moves.push_back(move);
        maneuver += ' ';
        maneuver += Move2Str[move];
    }
    return maneuver;
}

inline size_t count_maneuver_length(std::string_view maneuver) 
{
    if (maneuver.empty()) return 0;
    size_t count = 0;
    for (char c : maneuver) { if (c == ' ') count++; }
    return count + 1;
}

struct TestCase 
{
    std::string scramble;
    std::string config;
};

struct TestCaseResult 
{
    std::string scramble;
    std::string config;
    bool success;
    bool timeout;
    long long duration_ms;
    std::string solution_or_reason;
    size_t solution_len;

    static std::string format_header()
    {
        std::ostringstream oss;
        oss << std::right << std::setw(6)  << "Stat"
            << std::right << std::setw(12) << "Time"
            << std::right << std::setw(56) << "Cube Configuration"
            << std::right << std::setw(6)  << "Len"
            << "  " << "Solution / Reason";
        return oss.str();
    }

    std::string format_row(long long timeout_ms) const 
    {
        std::ostringstream oss;
        const std::string csep(1, ' ');

        if (timeout) {
            oss << std::right << std::setw(6)  << "[?]"
                << std::right << std::setw(12) << "--   "
                << std::right << std::setw(56) << config
                << std::right << std::setw(6)  << "--"
                << "  " << "[Exceeded " << timeout_ms << " ms limit]";
        } else if (success) {
            std::string time_str = std::to_string(duration_ms);
            oss << std::right << std::setw(6)  << "[O]"
                << std::right << std::setw(12) << time_str + " ms"
                << std::right << std::setw(56) << config
                << std::right << std::setw(6)  << solution_len
                << "  " << std::left  << solution_or_reason;
        } else {
            oss << std::right << std::setw(6)  << "[X]"
                << std::right << std::setw(12) << "--   "
                << std::right << std::setw(56) << config
                << std::right << std::setw(6)  << "--" 
                << "  " << std::left  << "[" << solution_or_reason << "]";
        }
        return oss.str();
    }
};

/*!
 * @brief Generate benchmark report on the performance of `solve`
 * 
 * @param N count of test cases
 * @param L length of scramble maneuver
 * @param G good enough length of solution
 * @param seed seed for rng
 * @param timeout_ms solve termination threshold
 * @param output_file output redirect (default: stdout)
 */
void benchmark_report(int N, int L, int G, int seed, long long timeout_ms, const std::string& output_file = "");