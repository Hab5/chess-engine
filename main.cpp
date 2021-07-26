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

template <EnumPiece Piece>
constexpr std::array<std::uint64_t, 64> Magics {};

template <>
constexpr std::array<std::uint64_t, 64> Magics<Rooks> {
    0x2080020500400f0ULL,
    0x28444000400010ULL,
    0x20000a1004100014ULL,
    0x20010c090202006ULL,
    0x8408008200810004ULL,
    0x1746000808002ULL,
    0x2200098000808201ULL,
    0x12c0002080200041ULL,
    0x104000208e480804ULL,
    0x8084014008281008ULL,
    0x4200810910500410ULL,
    0x100014481c20400cULL,
    0x4014a4040020808ULL,
    0x401002001010a4ULL,
    0x202000500010001ULL,
    0x8112808005810081ULL,
    0x40902108802020ULL,
    0x42002101008101ULL,
    0x459442200810c202ULL,
    0x81001103309808ULL,
    0x8110000080102ULL,
    0x8812806008080404ULL,
    0x104020000800101ULL,
    0x40a1048000028201ULL,
    0x4100ba0000004081ULL,
    0x44803a4003400109ULL,
    0xa010a00000030443ULL,
    0x91021a000100409ULL,
    0x4201e8040880a012ULL,
    0x22a000440201802ULL,
    0x30890a72000204ULL,
    0x10411402a0c482ULL,
    0x40004841102088ULL,
    0x40230000100040ULL,
    0x40100010000a0488ULL,
    0x1410100200050844ULL,
    0x100090808508411ULL,
    0x1410040024001142ULL,
    0x8840018001214002ULL,
    0x410201000098001ULL,
    0x8400802120088848ULL,
    0x2060080000021004ULL,
    0x82101002000d0022ULL,
    0x1001101001008241ULL,
    0x9040411808040102ULL,
    0x600800480009042ULL,
    0x1a020000040205ULL,
    0x4200404040505199ULL,
    0x2020081040080080ULL,
    0x40a3002000544108ULL,
    0x4501100800148402ULL,
    0x81440280100224ULL,
    0x88008000000804ULL,
    0x8084060000002812ULL,
    0x1840201000108312ULL,
    0x5080202000000141ULL,
    0x1042a180880281ULL,
    0x900802900c01040ULL,
    0x8205104104120ULL,
    0x9004220000440aULL,
    0x8029510200708ULL,
    0x8008440100404241ULL,
    0x2420001111000bdULL,
    0x4000882304000041ULL
};

template <>
constexpr std::array<std::uint64_t, 64> Magics<Bishops> {
    0x100420000431024ULL,
    0x280800101073404ULL,
    0x42000a00840802ULL,
    0xca800c0410c2ULL,
    0x81004290941c20ULL,
    0x400200450020250ULL,
    0x444a019204022084ULL,
    0x88610802202109aULL,
    0x11210a0800086008ULL,
    0x400a08c08802801ULL,
    0x1301a0500111c808ULL,
    0x1280100480180404ULL,
    0x720009020028445ULL,
    0x91880a9000010a01ULL,
    0x31200940150802b2ULL,
    0x5119080c20000602ULL,
    0x242400a002448023ULL,
    0x4819006001200008ULL,
    0x222c10400020090ULL,
    0x302008420409004ULL,
    0x504200070009045ULL,
    0x210071240c02046ULL,
    0x1182219000022611ULL,
    0x400c50000005801ULL,
    0x4004010000113100ULL,
    0x2008121604819400ULL,
    0xc4a4010000290101ULL,
    0x404a000888004802ULL,
    0x8820c004105010ULL,
    0x28280100908300ULL,
    0x4c013189c0320a80ULL,
    0x42008080042080ULL,
    0x90803000c080840ULL,
    0x2180001028220ULL,
    0x1084002a040036ULL,
    0x212009200401ULL,
    0x128110040c84a84ULL,
    0x81488020022802ULL,
    0x8c0014100181ULL,
    0x2222013020082ULL,
    0xa00100002382c03ULL,
    0x1000280001005c02ULL,
    0x84801010000114cULL,
    0x480410048000084ULL,
    0x21204420080020aULL,
    0x2020010000424a10ULL,
    0x240041021d500141ULL,
    0x420844000280214ULL,
    0x29084a280042108ULL,
    0x84102a8080a20a49ULL,
    0x104204908010212ULL,
    0x40a20280081860c1ULL,
    0x3044000200121004ULL,
    0x1001008807081122ULL,
    0x50066c000210811ULL,
    0xe3001240f8a106ULL,
    0x940c0204030020d4ULL,
    0x619204000210826aULL,
    0x2010438002b00a2ULL,
    0x884042004005802ULL,
    0xa90240000006404ULL,
    0x500d082244010008ULL,
    0x28190d00040014e0ULL,
    0x825201600c082444ULL
};

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

constexpr auto GetOccupancy(int index, std::uint64_t attack_mask) { // OK CONSTEXPR
    std::uint64_t occupancy = 0ULL;
    const auto mask_population = Utils::BitCount(attack_mask);
    for (int count = 0; count < mask_population; count++) {
        auto square = Utils::IndexLS1B(attack_mask);
        if (attack_mask  & (1ULL << square)) // PopSquare // TODO: Make this constexpr
            attack_mask ^= (1ULL << square);
        if (index & (1 << count))
            occupancy |= (1ULL << square);
    }
    return occupancy;
}

// std::mt19937_64 rng64(123456);

// template <EnumPiece Piece>
// constexpr auto FindMagicNumber(std::size_t square) noexcept { // Cannot be constexpr, but can be precalculated.
//     std::array<std::uint64_t, 4096> occupancies = { };
//     std::array<std::uint64_t, 4096> attacks = { };
//     std::array<std::uint64_t, 4096> used_attacks = { };
//     auto attack_mask = AttackMask<White, Piece>::On(square); // Ok constexpr
//     auto relevant_bits = AttackMaskBitCount<Piece>[square]; // Ok constexpr
//     int occupancy_idx = 1 << relevant_bits;

//     for (int idx = 0; idx < occupancy_idx; idx++) {
//         occupancies[idx] = GetOccupancy(idx, attack_mask); // Ok Constexpr
//         attacks[idx] = SliderAttacks<Piece>::On(square, occupancies[idx]); // Ok constexpr
//     }

//     for (int random_count = 0; random_count < 10000000; random_count++) {
//         std::uint64_t magic = rng64() & rng64() & rng64(); // could be constexpr
//         if (Utils::BitCount((attack_mask * magic) ^ 56) < 6) continue;
//         used_attacks.fill(0x00);
//         int idx, fail;
//         for (idx = 0, fail = 0; !fail && idx < occupancy_idx; idx++ ) {
//             std::uint64_t magic_idx = (occupancies[idx] * magic) >> (64 - relevant_bits);
//             if (used_attacks[magic_idx] == 0ULL)
//                 used_attacks[magic_idx] = attacks[idx];
//             else if (used_attacks[magic_idx] != attacks[idx]) fail = 1;
//         }

//         if (!fail) return magic;
//     }
//     std::cout << "magic failed\n";
//     return std::uint64_t(0);
// }

// template <EnumPiece Piece>
// constexpr auto GenerateMagicNumbers() noexcept {
//     std::array<std::uint64_t, 64> magics { };
//     for (int square = 0; square < 64; square++) {
//         magics[square] = FindMagicNumber<Piece>(square);
//     } return magics;
// }

template<EnumPiece Piece>
constexpr auto GetAttackMaskBitCount() {
    std::array<int, 64> MasksBitCount = { };
    for (int i = 0; i < 64; i++)
        MasksBitCount[i] = Utils::BitCount(AttackMask<White, Piece>::On(i));
    return MasksBitCount;
}

template <EnumPiece Piece>
constexpr std::array<int, 64> AttackMaskBitCount = GetAttackMaskBitCount<Piece>();

template <EnumPiece Piece>
[[nodiscard]] constexpr auto GenerateAttacks() noexcept { // OK
    std::array<std::array<std::uint64_t, 4096>, 64> attacks { };
    for (int square = 0; square < 64; square++) {
        auto attack_mask = AttackMask<White, Piece>::On(square);
        auto relevant_bits = AttackMaskBitCount<Piece>[square];
        int occupancy_idx = 1 << relevant_bits;
        for (int idx = 0; idx < occupancy_idx; idx++) {
            auto occupancy = GetOccupancy(idx, attack_mask);
            int magic_index = (occupancy * Magics<Piece>[square]) >> (64-relevant_bits);
            attacks[square][magic_index] = SliderAttacks<Piece>::On(square, occupancy);
        }
    } return attacks;
}

template <EnumPiece Piece>
constexpr std::array<std::array<std::uint64_t, 4096>, 64> Attacks = GenerateAttacks<Piece>();

template <EnumPiece Piece>
constexpr auto GetAttack(int square, std::uint64_t occupancy) { // OK
    occupancy &= AttackMask<White, Piece>::On(square);
    occupancy *= Magics<Piece>[square];
    occupancy >>= 64 - AttackMaskBitCount<Piece>[square];
    return Attacks<Piece>[square][occupancy];
}

int main() {
    Pieces Pieces;
    Utils::Print(GetAttack<Rooks>(e4, 0ULL | Utils::MakeSquare<e7>() | Utils::MakeSquare<g4>()));
    Utils::Print(GetAttack<Bishops>(e4, 0ULL | Utils::MakeSquare<c6>() | Utils::MakeSquare<g2>()));

}
