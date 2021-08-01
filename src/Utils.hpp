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
    #define Print(...) Utils::_Print(#__VA_ARGS__, __VA_ARGS__);
    static void _Print(const std::string& desc="None", Bitboard bitboard=0ULL) {
        std::stringstream ss;
        auto desc_size = int(desc.size()); desc_size += (desc_size < 25 ? 25-desc_size:0);
        for (int i = 0; i < desc_size+2; i++) ss << "─";
        auto line_padding = ss.str();

        auto empty_padding = [desc_size](int sz) -> std::string {
            std::stringstream ss;
            for (int pad = sz-desc_size-5; pad < 0; pad++) {
                ss << " "; if (pad+1 == 0) ss << "│";
            } return ss.str();
        };

        auto bitset = std::bitset<64>(Utils::Flip<Vertical>(bitboard));

        std::cout << "┌───────────────────┬" + line_padding + "┐";
        for (EnumSquare square = a1; square <= h8; ++square) {
            std::cout << (square % 8 ? " " : "\n│ " + std::to_string(8-square/8) + " ")
                      << (bitset[square] ? "■" : "□");
            if ((square+1) % 8 == 0) {
                std::cout << [&]() -> std::string {
                    ss.str(std::string());
                    switch (square/8) {
                    case  0: ss << "│ " + desc;                                 break;
                    case  1: ss << "├" << line_padding << "┤";                  break;
                    case  2: ss << std::hex << "│ HEX: " << bitset.to_ullong(); break;
                    case  3: ss << std::dec << "│ DEC: " << bitset.to_ullong(); break;
                    default: ss << "│"; break;
                    } return " " + ss.str() + empty_padding(ss.str().size());
                }() << std::dec;
            }
        } std::cout << "\n│ ϴ a b c d e f g h │" + empty_padding(3)
                    << "\n└───────────────────┴" + line_padding + "┘\n";
    }

    [[nodiscard]] static inline constexpr auto BitCount(Bitboard bitboard) noexcept {
        return static_cast<EnumSquare>(__builtin_popcountll(bitboard));
    }

    [[nodiscard]] static inline constexpr auto IndexLS1B(Bitboard bitboard) noexcept {
        return static_cast<EnumSquare>(bitboard ? __builtin_ctzll(bitboard):h8+1);
    }

    [[nodiscard]] static inline constexpr auto PopLS1B(Bitboard& bitboard) noexcept {
        auto tmp = static_cast<EnumSquare>(bitboard ? __builtin_ctzll(bitboard):h8+1);
        bitboard ^= tmp;
        return tmp;
    }

    [[nodiscard]] static constexpr auto MakeSquare(EnumSquare square) noexcept {
        return 0ULL | (1ULL << square);
    }

    template <EnumFlip Direction>
    [[nodiscard]] static constexpr Bitboard Flip(Bitboard bitboard) {
        constexpr auto flip_horizontal = [](Bitboard bitboard) noexcept {
            const Bitboard k1 = 0x5555555555555555;
            const Bitboard k2 = 0x3333333333333333;
            const Bitboard k4 = 0x0f0f0f0f0f0f0f0f;
            bitboard = ((bitboard >> 1) & k1) +  2*(bitboard & k1);
            bitboard = ((bitboard >> 2) & k2) +  4*(bitboard & k2);
            bitboard = ((bitboard >> 4) & k4) + 16*(bitboard & k4);
            return bitboard;
        };

        constexpr auto flip_diagonal = [](Bitboard bitboard) noexcept {
            return bitboard; // TODO
        };

        switch (Direction) {
        case Vertical  : return __builtin_bswap64(bitboard);
        case Horizontal: return flip_horizontal(bitboard);
        case Diagonal  : return flip_diagonal(bitboard);
        }
    }
};
