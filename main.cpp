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

std::uint64_t GetOccupancy(int index, std::uint64_t bitboard) {
    std::uint64_t occupancy = 0ULL;
    const auto mask_population = Utils::BitCount(bitboard);
    for (int count = 0; count < mask_population; count++) {
        auto square = Utils::IndexLS1B(bitboard);
        if (bitboard  & (1ULL << square)) // PopSquare
            bitboard ^= (1ULL << square);
        if (index & (1 << count))
            occupancy |= (1ULL << square);
    }
    return occupancy;
}

const int RooksOccupancyBitcount[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};
const int BishopsOccupancyBitcount[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6,
};

std::uint64_t GenerateMagicNumber() {
    std::mt19937_64 rng(time(0));
    return rng() & rng() & rng();
}

std::uint64_t FindMagicNumber(EnumSquare square) {
    std::array<std::uint64_t, 4096> occupancies = { };
    std::array<std::uint64_t, 4096> attacks = { };
    std::array<std::uint64_t, 4096> used_attacks = { };
    auto attack_mask = AttackMask<White, Rooks>::On(square);
    int occupancy_idx = 1 << RooksOccupancyBitcount[square];

    for (int idx = 0; idx < occupancy_idx; idx++) {
        occupancies[idx] = GetOccupancy(idx, attack_mask);
        attacks[idx] = SliderAttacks<Rooks>::On(square, occupancies[idx]);
    }

    for (int random_count = 0; random_count < 100000000; random_count++) {
        auto magic_number = GenerateMagicNumber();
        if (Utils::BitCount((attack_mask*magic_number) & 0xff00000000000000) < 6) continue;
        used_attacks.fill(0x00);
        int idx, fail;

        for (idx = 0, fail = 0; !fail && idx < occupancy_idx; idx++ ) {
            int magic_idx = (occupancies[idx] * magic_number) >> (64-RooksOccupancyBitcount[square]);
            if (used_attacks[magic_idx] == 0ULL)
                used_attacks[magic_idx] = attacks[idx];
            else if (used_attacks[magic_idx] != attacks[idx]) fail = 1;
        }

        if (!fail) return magic_number;
    }
    std::cout << "magic failed\n";
    return 0ULL;
}

int main() {
    Pieces Pieces;

   std::cout << FindMagicNumber(a1) << std::endl;
}
