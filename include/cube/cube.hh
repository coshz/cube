// SPDX-License-Identifier: MIT
// Copyright (c) 2026 coshz

#pragma once

#include "cube/cube_export.h"

#include <cstdint>
#include <string>
#include <string_view>

#ifndef __cplusplus
#error "This is a C++ header; please use a C++ compiler OR include cube/cube.h instead"
#endif 

namespace cube {

// The identity of color configuration
constexpr std::string_view cubeId = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

enum class SolveResult : int32_t {
    Success       = 0,
    Unsolvable    = 1,
    NotFound      = 2,
    InvalidSrc    = 3,
    InvalidTgt    = 4,
    UnknownErr    = 5
};

CUBE_EXPORT constexpr std::string_view to_string(SolveResult status) noexcept {
    switch (status) {
    case SolveResult::Success:      return "Success";
    case SolveResult::Unsolvable:   return "The cube configuration is unsolvable";
    case SolveResult::NotFound:     return "No solution found within the step limit";
    case SolveResult::InvalidSrc:   return "Invalid source color configuration";
    case SolveResult::InvalidTgt:   return "Invalid target color configuration";
    case SolveResult::UnknownErr:   return "Unknown error";                       
    }
}

enum class PermFormat : int {
    Face  = 0,
    Cubie = 1,
    Cycle = 2
};

struct Solution {
    SolveResult status;
    std::string maneuver;

    bool is_success() const noexcept {
        return status == SolveResult::Success;
    }
};

/*!
 * @brief Checks if a color configuration is solvable.
 */
CUBE_EXPORT bool is_solvable(std::string_view color_cube);

/*!
 * @brief Solves the Rubik's cube from source state to target state.
 */
[[nodiscard]] CUBE_EXPORT Solution solve(
    std::string_view src,
    std::string_view tgt = cubeId,
    int step = 30,
    bool best = true
);

/*!
 * @brief Applies a sequence of moves to a color cube.
 * @throws std::invalid_argument if the configuration or maneuver is invalid.
 */
[[nodiscard]] CUBE_EXPORT std::string apply_maneuver(
    std::string_view maneuver,
    std::string_view cube = cubeId
);

/*!
 * @brief Computes the permutation state resulting from a maneuver or a color cube.
 * @throws std::invalid_argument if the input is invalid.
 */
[[nodiscard]] CUBE_EXPORT std::string show_permutation(
    std::string_view ms_or_cube,
    PermFormat format = PermFormat::Cycle
);

} // namespace cube