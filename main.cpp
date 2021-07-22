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

struct Pieces final {
public:

    [[nodiscard]] inline std::uint64_t& operator[](std::size_t query) {
        return all[query];
    }

    [[nodiscard]] inline std::uint64_t operator[](std::size_t query) const {
    return all[query];
}

template<EnumColor Color>
[[nodiscard]] inline bool GetSquare(EnumSquare sq) const {
    return all[Color] & (1ULL << sq);
}

template<EnumColor Color, EnumPiece Piece>
inline void SetSquare(EnumSquare sq) {
    all[Piece]  |= (1ULL << sq);
    all[Color] |= (1ULL << sq);
}

template<EnumColor Color, EnumPiece Piece>
inline void PopSquare(EnumSquare sq) {
    if (all[Piece] & (1ULL << sq)) {
        all[Piece]  ^= (1ULL << sq);
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
    for (auto square = 0; square < 64; square++)
    Utils::Print(Attack<White, Rooks>::On(square));
    // Utils::Print(Pieces[White] & Pieces[Pawns] & Rank_2);
    // Utils::Print(Attack<White, Rooks>::At(a1));
}
