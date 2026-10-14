#include "bench.h"
#include "teestream.h"
#include "cube/cube.hh"
#include "cube/version.h"
#include <chrono>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <random>
#include <iomanip>
#include <algorithm>
#include <future>
#include <sstream>
#include <filesystem>

using namespace cube;
using namespace std::chrono_literals;

std::vector<TestCase> generate_test_cases(int N, int L, unsigned int seed)
{
    std::mt19937 rng(seed);
    std::vector<TestCase> cases;
    cases.reserve(N);
    for (int i = 0; i < N; ++i) 
    {
        std::string scramble = generate_random_maneuver(L, rng);
        std::string config = cube::apply_maneuver(scramble);
        cases.push_back({scramble, config});
    }
    return cases;
}

std::string timestamp(const char* fmt = "%Y-%m-%d %H:%M:%S") 
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char time_buf[64];
    std::strftime(time_buf, sizeof(time_buf), fmt, std::localtime(&now_c));
    return time_buf;
}

std::string out_filename(std::string tag, int N, int L, int G, unsigned int S, long long T)
{
    std::stringstream ss;
    ss << tag 
       << "_N" << N
       << "_L" << L
       << "_G" << G
       << "_S" << S
       << "_T" << T
       << "_v" << std::hex << std::setfill('0') << std::setw(4) << CUBE_VERSION_CODE
       << ".txt"
    ;
    return ss.str();
}

void benchmark_report(int N, int L, int G, unsigned int seed, long long timeout_ms, const std::string& log_file = "") 
{
    TeeStream os(log_file);
    std::vector<TestCase> test_cases = generate_test_cases(N, L, seed);

    if (N == 0) { os << "No test cases provided." << std::endl; return; }

    os << "================================================================================\n";
    os << "                  RUBIK'S CUBE SOLVER PERFORMANCE BENCHMARK                     \n";
    os << "================================================================================\n";
    os << " • Library Version       : " << CUBE_VERSION << "\n";
    os << " • Generated At          : " << timestamp("%Y-%m-%d %H:%M:%S") << "\n";
    os << " • Test Cases (N)        : " << N << "\n";
    os << " • Scramble Len (L)      : " << L << "\n";
    os << " • Goodma Threshold (G)  : " << G << "\n";
    os << " • Random Seed           : " << seed << "\n";
    os << " • Timeout Threshold     : " << timeout_ms << " ms\n";
    os << "--------------------------------------------------------------------------------\n";
    os << std::flush;

    os << TestCaseResult::format_header() << std::endl;
    os << "--------------------------------------------------------------------------------\n";
    
    std::vector<TestCaseResult> results;
    results.reserve(N);

    long long total_ms = 0;
    long long min_ms = -1;
    long long max_ms = 0;
    int success_count = 0;
    int timeout_count = 0;
    int fail_count = 0;

    std::vector<long long> valid_durations;
    valid_durations.reserve(N);

    for (int i = 0; i < N; ++i) 
    {
        const auto& [scramble, config] = test_cases[i];

        auto solve_task = std::async(std::launch::async, [G,c=config]() {
            return time_execution(solve, c, cubeId, G > 0 ? G : 0);
        });

        if (solve_task.wait_for(timeout_ms * 1ms) == std::future_status::timeout) {
            timeout_count++;
            TestCaseResult res{scramble, config, false, true, 0, "[TIMEOUT]", 0};
            results.push_back(res);
            os << res.format_row(timeout_ms) << std::endl;
            continue;
        }

        auto [dur, opt_res] = solve_task.get();
        long long duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

        bool success = opt_res && opt_res->is_success();
        
        if (success) {
            success_count++;
            std::string maneuver = opt_res->maneuver;
            size_t sol_len = count_maneuver_length(maneuver);

            valid_durations.push_back(duration_ms);
            total_ms += duration_ms;
            if (min_ms == -1 || duration_ms < min_ms) min_ms = duration_ms;
            if (duration_ms > max_ms) max_ms = duration_ms;

            TestCaseResult res{scramble, config, true, false, duration_ms, maneuver, sol_len};
            results.push_back(res);
            os << res.format_row(timeout_ms) << std::endl;
        } else {
            fail_count++;
            std::string reason = opt_res ? std::string(cube::to_string(opt_res->status)) : "Unknown Error";
            TestCaseResult res{scramble, config, false, false, duration_ms, reason, 0};
            results.push_back(res);
            os << res.format_row(timeout_ms) << std::endl;
        }
    }

    os << "--------------------------------------------------------------------------------\n";
    
    if (valid_durations.empty()) {
        os << " [Error] No valid successful cases to summarize.\n\n";
        return;
    }

    std::sort(valid_durations.begin(), valid_durations.end());
    long long median_ms = valid_durations[valid_durations.size() / 2];
    long long avg_ms = total_ms / valid_durations.size();
    double success_rate = (static_cast<double>(success_count) / N) * 100.0;

    os << "\n";
    os << "================================================================================\n";
    os << "                               BENCHMARK SUMMARY                                \n";
    os << "================================================================================\n";
    os << std::fixed << std::setprecision(2);
    os << " • Total Cases    : " << N << "\n";
    os << " • Successful     : " << success_count << " (" << success_rate << "%)\n";
    os << " • Timeouts       : " << timeout_count << "\n";
    os << " • Failed/Other   : " << fail_count << "\n";
    os << "--------------------------------------------------------------------------------\n";
    os << " • Average Time   : " << std::right << std::setw(7) << avg_ms    << " ms\n";
    os << " • Median Time    : " << std::right << std::setw(7) << median_ms << " ms\n";
    os << " • Min Time       : " << std::right << std::setw(7) << min_ms    << " ms\n";
    os << " • Max Time       : " << std::right << std::setw(7) << max_ms    << " ms\n";
    os << "================================================================================\n";
}

static void print_help(const char* program_name) 
{
    std::cout << "Usage: " << program_name << " [options]\n\n"
              << "Options:\n"
              << "  -n <num>      Number of test cases (default: 100)\n"
              << "  -l <len>      Scramble length (default: 30)\n"
              << "  -g <len>      Goodma Threshold (default: 22)\n"
              << "  -t <ms>       Timeout threshold in milliseconds (default: 20000)\n"
              << "  -s <seed>     Random seed (default: 1337)\n"
              << "  -o <file>     Output log file (default: bench.log)\n"
              << "  -h            Show this help message\n";
}

int main(int argc, char* argv[]) 
{
    int N = 100;
    int L = 30;
    int G = 22;
    long long timeout_ms = 10'000;
    unsigned int seed = 1337;
    std::string outfile {};
    for (int i = 1; i < argc; ++i) 
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        }
        if (i + 1 < argc) {
            if (arg == "-n") {
                N = std::stoi(argv[++i]);
            } else if (arg == "-l") {
                L = std::stoi(argv[++i]);
            } else if (arg == "-g") {
                G = std::stoi(argv[++i]);
            } else if (arg == "-t") {
                timeout_ms = std::stoll(argv[++i]);
            } else if (arg == "-s") {
                seed = static_cast<unsigned int>(std::stoul(argv[++i]));
            } else if (arg == "-o") {
                outfile = argv[++i];
            }
        }
    }
    if(outfile.empty()) { outfile = out_filename("solver",N,L,G,seed,timeout_ms); }

    std::cout << "Generating benchmark report...\n" << std::endl;
    benchmark_report(N, L, G, seed, timeout_ms, outfile);
    std::cout << "\nDone. Report Location: " << std::filesystem::canonical(outfile).string() << std::endl;
}
