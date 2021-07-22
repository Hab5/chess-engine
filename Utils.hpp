#pragma once

#include "ChessEngine.hpp"

#include <bitset>
#include <string>
#include <iostream>
#include <array>
#include <sstream>

enum EnumFlip: std::uint8_t {
    Vertical,
    Horizontal,
    Diagonal
};

class Utils final {
public:
    #define Print(...) Utils::_Print(__VA_ARGS__, #__VA_ARGS__); // good luck
    static void _Print(std::uint64_t bitboard, const std::string& desc="None") {
        std::stringstream ss;
        auto bitset = std::bitset<64>(Utils::Flip<Vertical>(bitboard));
        auto desc_size = int(desc.size()); desc_size += (desc_size < 24 ? 24-desc_size:0);
        for (int i = 0; i < desc_size+2; i++) ss << "─";
        auto line_padding = ss.str();

        auto empty_padding = [desc_size](int sz) -> std::string {
            std::stringstream ss;
            for (int pad = sz-desc_size-5; pad < 0; pad++) {
                ss << " "; if (pad+1 == 0) ss << "│";
            } return ss.str();
        };

        std::cout << "┌───────────────────┬" + line_padding + "┐";
        for (int i = 0; i < 64;) {
            std::cout << (i % 8 ? "" : "\n│ " + std::to_string(8-i/8) + " ")
                      << (bitset[i] ? "■ " : "□ ");
            if (++i % 8 == 0) {
                std::cout << [&]() -> std::string {
                    ss.str(std::string());
                    switch (i/8) {
                    case  1: ss << "│ " + desc; break;
                    case  2: ss << "├" << line_padding << "┤"; break;
                    case  3: ss << std::hex << "│ HEX: " << bitset.to_ullong(); break;
                    case  4: ss << std::dec << "│ DEC: " << bitset.to_ullong(); break;
                    default: ss << "│"; break;
                    } ss << empty_padding(ss.str().size());
                    return ss.str();
                }() << std::dec;
            }
        } std::cout << "\n│   a b c d e f g h │" + empty_padding(3)
                    << "\n└───────────────────┴" + line_padding + "┘\n";
    }

    template <EnumSquare Square>
    [[nodiscard]] static constexpr bool GetSquare(std::uint64_t bitboard) noexcept {
        return bitboard & (1ULL << Square);
    }

    template <EnumSquare Square>
    [[nodiscard]] static constexpr std::uint64_t SetSquare(std::uint64_t bitboard) noexcept {
        return bitboard |= (1ULL << Square);
    }

    template <EnumSquare Square>
    [[nodiscard]] static constexpr std::uint64_t PopSquare(std::uint64_t bitboard) noexcept {
        if (bitboard  & (1ULL << Square))
            bitboard ^= (1ULL << Square);
        return bitboard;
    }

    template <EnumSquare Square>
    [[nodiscard]] static constexpr std::uint64_t MakeSquare() noexcept {
        return 0ULL | (1ULL << Square);
    }

    template <EnumFlip Direction>
    [[nodiscard]] static constexpr std::uint64_t Flip(std::uint64_t bitboard) {
        constexpr auto flip_horizontal = [](std::uint64_t bitboard) noexcept {
            const std::uint64_t k1 = 0x5555555555555555;
            const std::uint64_t k2 = 0x3333333333333333;
            const std::uint64_t k4 = 0x0f0f0f0f0f0f0f0f;
            bitboard = ((bitboard >> 1) & k1) +  2*(bitboard & k1);
            bitboard = ((bitboard >> 2) & k2) +  4*(bitboard & k2);
            bitboard = ((bitboard >> 4) & k4) + 16*(bitboard & k4);
            return bitboard;
        };

        constexpr auto flip_diagonal = [](std::uint64_t bitboard) noexcept {
            return bitboard; // TODO
        };

        switch (Direction) {
        case Vertical  : return __builtin_bswap64(bitboard);
        case Horizontal: return flip_horizontal(bitboard);
        case Diagonal  : return flip_diagonal(bitboard);
        }
    }
};
