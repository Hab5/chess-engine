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

    template<EnumColor color>
    [[nodiscard]] inline bool GetSquare(EnumSquare sq) const {
        return all[color] & (1ULL << sq);
    }

    template<EnumColor color, EnumPiece type>
    inline void SetSquare(EnumSquare sq) {
        all[type]  |= (1ULL << sq);
        all[color] |= (1ULL << sq);
    }

    template<EnumColor color, EnumPiece type>
    inline void PopSquare(EnumSquare sq) {
        if (all[type] & (1ULL << sq)) {
            all[type]  ^= (1ULL << sq);
            all[color] ^= (1ULL << sq);
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
    Utils::Print((Pieces[White] & Pieces[Knights]) & File_B);
}
