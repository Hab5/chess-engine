#pragma once

#include "ChessEngine.hpp"

#include <bitset>
#include <string>
#include <iostream>
#include <array>

enum EnumFlip: std::uint8_t {
    Vertical,
    Horizontal,
    Diagonal
};

namespace Utils {

void Print(std::uint64_t bitboard) noexcept;

template <EnumFlip Direction>
[[nodiscard]] constexpr std::uint64_t Flip(std::uint64_t bitboard) noexcept;

template <EnumSquare sq>
[[nodiscard]] constexpr bool GetSquare(std::uint64_t bitboard) noexcept;

template <EnumSquare sq>
[[nodiscard]] static constexpr auto SetSquare(std::uint64_t bitboard) noexcept;

template <EnumSquare sq>
[[nodiscard]] constexpr auto PopSquare(std::uint64_t bitboard) noexcept;

template <EnumSquare sq>
[[nodiscard]] constexpr auto MakeSquare() noexcept;

}
