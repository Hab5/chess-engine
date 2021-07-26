#include "ChessEngine.hpp"
#include "Attack.hpp"
#include "Utils.hpp"

#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <functional>
#include <numeric>
#include <stdlib.h>
#include <type_traits>
#include <chrono>
#include <random>

struct Pieces final {
public:

    [[nodiscard]] inline std::uint64_t& operator[](std::size_t query) noexcept {
        return all[query];
    }

    [[nodiscard]] inline std::uint64_t operator[](std::size_t query) const noexcept {
        return all[query];
    }

    template<EnumColor Color>
    [[nodiscard]] inline bool GetSquare(EnumSquare sq) const noexcept {
        return all[Color] & (1ULL << sq);
    }

    template<EnumColor Color, EnumPiece Piece>
    inline void SetSquare(EnumSquare sq) noexcept{
        all[Piece] |= (1ULL << sq);
        all[Color] |= (1ULL << sq);
    }

    template<EnumColor Color, EnumPiece Piece>
    inline void PopSquare(EnumSquare sq) noexcept {
        if (all[Piece]  & (1ULL << sq)) {
            all[Piece] ^= (1ULL << sq);
            all[Color] ^= (1ULL << sq);
        }
    }

private:
    std::array<std::uint64_t, 8> all { // Default
        (0xffffULL << 0 ) | 0x0000ULL, // White
        (0xffffULL << 48) | 0x0000ULL, // Black
        (0xff00ULL << 40) | 0xff00ULL, // Pawns
        (0x0042ULL << 56) | 0x0042ULL, // Knights
        (0x0024ULL << 56) | 0x0024ULL, // Bishops
        (0x0081ULL << 56) | 0x0081ULL, // Rooks
        (0x0008ULL << 56) | 0x0008ULL, // Queens
        (0x0010ULL << 56) | 0x0010ULL  // King
    };
};

int main() {
    Pieces Pieces;

    Utils::Print(Attack<Rooks>::On(a1, Utils::MakeSquare<a6>() | Utils::MakeSquare<f1>()));
    Utils::Print(Attack<Bishops>::On(a1, Utils::MakeSquare<d4>() | Utils::MakeSquare<f1>()));
}
