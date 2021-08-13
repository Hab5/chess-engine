#pragma once

#include "ChessEngine.hpp"
#include "Utils.hpp"

// #define CONSTEXPR_MAGIC_BITBOARD

#ifndef CONSTEXPR_MAGIC_BITBOARD
#define _constexpr inline
#else
#define _constexpr constexpr
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// ATTACK GENERATOR //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

namespace Generator {

template <auto... Args>
class Attacks final { };

/////////////////////////////////////////// PAWNS /////////////////////////////////////////////

template <EnumColor Color, EnumPiece Piece>
class Attacks<Color, Piece> final {
public:
    [[nodiscard]] static constexpr auto Get() noexcept {
        return Attacks::Pawn();
    }

private:
    [[nodiscard]] static constexpr auto Pawn() noexcept {
        std::array<Bitboard, 64> attacks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            Bitboard pawn = Utils::MakeSquare(square);
            attacks[square] = {
                  Utils::ShiftTo<(Color == White ? North:South)|East>(pawn)
                | Utils::ShiftTo<(Color == White ? North:South)|West>(pawn)
            };
        } return attacks;
    }

     Attacks() = delete;
    ~Attacks() = delete;
};

/////////////////////////////////////// KNIGHTS / KING ////////////////////////////////////////

template <EnumPiece Piece>
class Attacks<Piece> final {
public:
    [[nodiscard]] static constexpr auto Get() noexcept {
        if constexpr(Piece == EnumPiece::Knights) return Attacks::Knight();
        if constexpr(Piece == EnumPiece::King   ) return Attacks::King();
    }

private:
    [[nodiscard]] static constexpr auto Knight() noexcept {
        std::array<Bitboard, 64> attacks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            Bitboard knight = Utils::MakeSquare(square);
            attacks[square] = {
                  Utils::ShiftTo<North|North|West>(knight)
                | Utils::ShiftTo<North|North|East>(knight)
                | Utils::ShiftTo<South|South|West>(knight)
                | Utils::ShiftTo<South|South|East>(knight)
                | Utils::ShiftTo<North| West|West>(knight)
                | Utils::ShiftTo<North| East|East>(knight)
                | Utils::ShiftTo<South| West|West>(knight)
                | Utils::ShiftTo<South| East|East>(knight)
            };
        } return attacks;
    }

    [[nodiscard]] static constexpr auto King() noexcept {
        std::array<Bitboard, 64> attacks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            Bitboard king    = Utils::MakeSquare(square);
            attacks[square] = {
                  Utils::ShiftTo<North     >(king)
                | Utils::ShiftTo<East      >(king)
                | Utils::ShiftTo<West      >(king)
                | Utils::ShiftTo<South     >(king)
                | Utils::ShiftTo<North|West>(king)
                | Utils::ShiftTo<North|East>(king)
                | Utils::ShiftTo<South|West>(king)
                | Utils::ShiftTo<South|East>(king)
            };
        } return attacks;
    }

     Attacks() = delete;
    ~Attacks() = delete;
};

///////////////////////////////////////// BISHOPS /////////////////////////////////////////////

template <>
class Attacks<Bishops> final {
public:

    [[nodiscard]] static constexpr auto MaskTable() noexcept {
        std::array<Bitboard, 64> masks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            int tr = square / 8, tf = square % 8; // 2D Square Index
            #define SET_SQUARE masks[square] |= EnumSquare(f+r*8);
            for (int r = tr+1, f = tf+1; r <= 6 && f <= 6; r++,f++) SET_SQUARE // NE
            for (int r = tr+1, f = tf-1; r <= 6 && f >= 1; r++,f--) SET_SQUARE // NW
            for (int r = tr-1, f = tf+1; r >= 1 && f <= 6; r--,f++) SET_SQUARE // SE
            for (int r = tr-1, f = tf-1; r >= 1 && f >= 1; r--,f--) SET_SQUARE // SW
            #undef  SET_SQUARE
        } return masks;
    }

    [[nodiscard]] static _constexpr auto MaskTableBitCount() noexcept {
        std::array<Bitboard, 64> masks = MaskTable();
        std::array<int, 64> masks_bitcount = { };
        for (EnumSquare square = a1; square <= h8; ++square)
            masks_bitcount[square] = Utils::BitCount(masks[square]);
        return masks_bitcount;
    }

    [[nodiscard]] static _constexpr auto AttackTable() noexcept {
        auto get_attack = [](EnumSquare square, Bitboard occupancy) constexpr {
            Bitboard attack = 0ULL, b = 0ULL, o = occupancy;
            int tr = square / 8, tf = square % 8; // 2D Square Index
            #define SET_SQUARE { b=0; b|=EnumSquare(f+r*8); attack|=b; if (b&o) break; }
            for (int r = tr+1, f = tf+1; r <= 7 && f <= 7; r++,f++) SET_SQUARE // NE
            for (int r = tr+1, f = tf-1; r <= 7 && f >= 0; r++,f--) SET_SQUARE // NW
            for (int r = tr-1, f = tf+1; r >= 0 && f <= 7; r--,f++) SET_SQUARE // SE
            for (int r = tr-1, f = tf-1; r >= 0 && f >= 0; r--,f--) SET_SQUARE // SW
            #undef  SET_SQUARE
            return attack;
        };

        auto get_occupancy = [](int index, Bitboard attack_mask) constexpr {
            Bitboard occupancy = 0ULL;
            const auto mask_population = Utils::BitCount(attack_mask);
            for (int count = 0; count < mask_population; count++) {
                auto square = Utils::PopLS1B(attack_mask);
                if (index & (1 << count))
                    occupancy |= (1ULL << square);
            } return occupancy;
        };

        std::array<Bitboard, 64> masks      = MaskTable();
        std::array<int, 64> masks_bitcount  = MaskTableBitCount();
        std::array<std::array<Bitboard, 512>, 64> attacks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            auto attack_mask = masks[square];
            auto relevant_bits = masks_bitcount[square];
            int occupancy_idx = 1 << relevant_bits;
            for (int idx = 0; idx < occupancy_idx; idx++) {
                auto occupancy = get_occupancy(idx, attack_mask);
                int magic_idx = (occupancy * _MagicNumbers[square]) >> (64-9);
                attacks[square][magic_idx] = get_attack(square, occupancy);
            }
        } return attacks;
    }

    [[nodiscard]] static _constexpr auto MagicNumbers() noexcept { return _MagicNumbers; }

    [[nodiscard]] static _constexpr auto Magics_AOS() noexcept {
        struct Magic {
            Bitboard                   Mask;
            std::uint64_t              Number;
            std::array<Bitboard, 512>  Attack;
        };

        auto masks   = Generator::Attacks<Bishops>::MaskTable();
        auto numbers = Generator::Attacks<Bishops>::MagicNumbers();
        auto attacks = Generator::Attacks<Bishops>::AttackTable();

        std::array<Magic, 64> Magics;
        for (EnumSquare square = a1; square <= h8; ++square) {
            Magics[square] = Magic {
                .Mask   = masks[square],
                .Number = numbers[square],
                .Attack = attacks[square]
            };
        } return Magics;
    }

    [[nodiscard]] static _constexpr auto Magics_SOA() noexcept {
        struct Magics {
            const std::array<std::array<Bitboard, 512>, 64>  Attacks;
            const std::array<std::uint64_t, 64>              Numbers;
            const std::array<Bitboard, 64>                   Masks;
        };

        return Magics {
            .Attacks = Generator::Attacks<Bishops>::AttackTable(),
            .Numbers = Generator::Attacks<Bishops>::MagicNumbers(),
            .Masks   = Generator::Attacks<Bishops>::MaskTable(),
        };
    }


private:
    static _constexpr std::array<std::uint64_t, 64> _MagicNumbers {
        0x8062200800306044ULL, 0x12081c800408a0ULL,   0x201043231881009ULL,  0x2423300880040300ULL,
        0x4004140d12000060ULL, 0xa4244050c6800100ULL, 0x801a00c4040402c0ULL, 0x42020009042c0102ULL,
        0x250005133002080ULL,  0x2c8002090108008ULL,  0x181014300881000ULL,  0x28a0802641006124ULL,
        0x1010050142120200ULL, 0x108a112004000ULL,    0x440200108100ULL,     0x1208000200310224ULL,
        0x1004a4052c02081ULL,  0x4001200108020422ULL, 0x80a880088010008ULL,  0x200204864001400ULL,
        0x1801154012220002ULL, 0x4092006420802818ULL, 0x4200048089082ULL,    0x1008084010241400ULL,
        0x203010a0008018ULL,   0xcc02004040800ULL,    0x470008024400ULL,     0x8420080002081010ULL,
        0x10084001080a000ULL,  0x8011022410404a02ULL, 0x2006044083100ULL,    0x9401000010060aULL,
        0x20452000081800ULL,   0x811004800102a01ULL,  0x86220804800ULL,      0x1100100820740400ULL,
        0x1010400120220ULL,    0x420080021104402ULL,  0x1401043024220882ULL, 0x4000c00900080c00ULL,
        0x1080802022902ULL,    0xa8842428001000ULL,   0x6540428040302400ULL, 0x8000201080800800ULL,
        0x900020212304308ULL,  0x10908082000008ULL,   0x284040010900201ULL,  0x69240a30a04040ULL,
        0x8230181609100010ULL, 0x11c0c0210b27320ULL,  0x300808070044ULL,     0x84200080848a0902ULL,
        0x8020012414214000ULL, 0x9022999820000ULL,    0x400880c5801a104ULL,  0x3620081a42004000ULL,
        0x84004d804d10820ULL,  0x2101010040202002ULL, 0x8000400160241020ULL, 0x42100000010222a0ULL,
        0x1802022020200c0ULL,  0x200000490881200ULL,  0x30000480529101c0ULL, 0x100a803082114022ULL,
    };
     Attacks() = delete;
    ~Attacks() = delete;
};

////////////////////////////////////////// ROOKS //////////////////////////////////////////////

template <>
class Attacks<Rooks> final {
public:
    [[nodiscard]] static _constexpr auto MaskTable() noexcept {
        std::array<Bitboard, 64> masks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            int tr = square / 8, tf = square % 8; // 2D Square Index
            #define SET_SQUARE masks[square] |= EnumSquare(f+r*8);
            for (int r = tr+1, f = tf;   r <= 6; r++) SET_SQUARE // N
            for (int r = tr-1, f = tf;   r >= 1; r--) SET_SQUARE // S
            for (int r = tr,   f = tf+1; f <= 6; f++) SET_SQUARE // E
            for (int r = tr,   f = tf-1; f >= 1; f--) SET_SQUARE // W
            #undef  SET_SQUARE
        } return masks;
    }

    [[nodiscard]] static _constexpr auto MaskTableBitCount() noexcept {
        std::array<Bitboard, 64> masks = MaskTable();
        std::array<int, 64> masks_bitcount = { };
        for (EnumSquare square = a1; square <= h8; ++square)
            masks_bitcount[square] = Utils::BitCount(masks[square]);
        return masks_bitcount;
    }

    [[nodiscard]] static _constexpr auto AttackTable() noexcept {
        auto get_attack = [](EnumSquare square, Bitboard occupancy) constexpr {
            Bitboard a = 0ULL, rk = 0ULL, o = occupancy;
            int tr = square / 8, tf = square % 8;
            #define SET_SQUARE { rk=0; rk|=EnumSquare(f+r*8); a|=rk; if (rk&o) break; }
            for (int r = tr+1, f = tf;   r <= 7; r++) SET_SQUARE // N
            for (int r = tr-1, f = tf;   r >= 0; r--) SET_SQUARE // S
            for (int r = tr,   f = tf+1; f <= 7; f++) SET_SQUARE // E
            for (int r = tr,   f = tf-1; f >= 0; f--) SET_SQUARE // W
            #undef  SET_SQUARE
            return a;
        };

        auto get_occupancy = [](int index, Bitboard attack_mask) constexpr {
            Bitboard occupancy = 0ULL;
            const auto mask_population = Utils::BitCount(attack_mask);
            for (int count = 0; count < mask_population; count++) {
                auto square = Utils::PopLS1B(attack_mask);
                if (index & (1 << count))
                    occupancy |= (1ULL << square);
            } return occupancy;
        };

        std::array<Bitboard, 64> masks = MaskTable();
        std::array<int, 64> masks_bitcount  = MaskTableBitCount();
        std::array<std::array<Bitboard, 4096>, 64> attacks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            auto attack_mask = masks[square];
            auto relevant_bits = masks_bitcount[square];
            int occupancy_idx = 1 << relevant_bits;
            for (int idx = 0; idx < occupancy_idx; idx++) {
                auto occupancy = get_occupancy(idx, attack_mask);
                int magic_idx = (occupancy * _MagicNumbers[square]) >> (64-12);
                attacks[square][magic_idx] = get_attack(square, occupancy);
            }
        } return attacks;
    }

    [[nodiscard]] static _constexpr auto MagicNumbers() noexcept { return _MagicNumbers; }

    [[nodiscard]] static _constexpr auto Magics_AOS() {
        struct Magic {
            Bitboard                   Mask;
            std::uint64_t              Number;
            std::array<Bitboard, 4096> Attack;
        };

        auto masks   = Generator::Attacks<Rooks>::MaskTable();
        auto numbers = Generator::Attacks<Rooks>::MagicNumbers();
        auto attacks = Generator::Attacks<Rooks>::AttackTable();

        std::array<Magic, 64> Magics;
        for (EnumSquare square = a1; square <= h8; ++square) {
            Magics[square] = Magic {
                .Mask   = masks[square],
                .Number = numbers[square],
                .Attack = attacks[square]
            };
        } return Magics;
    }

    [[nodiscard]] static _constexpr auto Magics_SOA() {
        struct Magics {
            const std::array<std::array<Bitboard, 4096>, 64> Attacks;
            const std::array<std::uint64_t, 64>              Numbers;
            const std::array<Bitboard, 64>                   Masks;
        };

        return Magics {
            .Attacks = Generator::Attacks<Rooks>::AttackTable(),
            .Numbers = Generator::Attacks<Rooks>::MagicNumbers(),
            .Masks   = Generator::Attacks<Rooks>::MaskTable(),
        };
    }

private:
    static _constexpr std::array<std::uint64_t, 64> _MagicNumbers {
        0xd800010804000a0ULL,  0x40004820807041ULL,   0x20040008026008ULL,   0x100408040a8010ULL,
        0x500100800010a04ULL,  0x400880210200400ULL,  0x208010c221000080ULL, 0x1100018150210002ULL,
        0x42040200200080ULL,   0x80110804000900a8ULL, 0xb0080c0042000ULL,    0x802000202284188ULL,
        0x511000208101100ULL,  0x4400084400101ULL,    0x8044100900004020ULL, 0x580424080004100ULL,
        0x6040a00122100aULL,   0x6800282001100120ULL, 0x2110050100102c04ULL, 0x44002800201088ULL,
        0x1000020880100409ULL, 0x1130040080100ULL,    0x10424040010d80ULL,   0x8004088000285300ULL,
        0x2024200109020ULL,    0x802b01010244a408ULL, 0x180040020000801ULL,  0x1010022200124008ULL,
        0x6080300108042800ULL, 0x4001004040098ULL,    0x244181024100ULL,     0x4201000840002080ULL,
        0x4244002000201000ULL, 0x6000102040840028ULL, 0x808800408801000ULL,  0x1000080530800220ULL,
        0x4000800841044040ULL, 0x80c002120080842ULL,  0x940300284310a00ULL,  0x10101c0200c71ULL,
        0x2088002040002012ULL, 0x10008102002a0046ULL, 0x8008030460200200ULL, 0x1060210300090010ULL,
        0x8430c0802601800ULL,  0x448c800308400eULL,   0x20000e0002010990ULL, 0x2080004000200808ULL,
        0x4840119310c45200ULL, 0x4000108041000880ULL, 0x10042000425100ULL,   0x11122010042020ULL,
        0x202020010042040ULL,  0x2841100080222050ULL, 0x55000080104010ULL,   0x2200020092006040ULL,
        0x2010290280420012ULL, 0x800080201040000fULL, 0x300500502402001ULL,  0x40510084200a202ULL,
        0x4801480010010605ULL, 0x4000888020401ULL,    0x2106e08010084ULL,    0x4081040061418102ULL,
    };
    Attacks() = delete;
   ~Attacks() = delete;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// MAGIC NUMBER GENERATION ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

// #include "ChessEngine.hpp"
// #include "Attack.hpp"
// #include "Utils.hpp"
// #include <random>

// auto GetOccupancy(int index, Bitboard attack_mask) {
//     Bitboard occupancy = 0ULL;
//     const auto mask_population = Utils::BitCount(attack_mask);
//     for (int count = 0; count < mask_population; count++) {
//         auto square = Utils::PopLS1B(attack_mask);
//         if (index & (1 << count))
//             occupancy |= (1ULL << square);
//     } return occupancy;
// }
// std::mt19937_64 rng64(123456);

// template <EnumPiece Piece>
// auto FindMagicNumber(EnumSquare square) noexcept {
//     constexpr auto FixedShift = Piece == Bishops ? 64-9 : 64-12;
//     std::array<Bitboard, 4096> occupancies = { };
//     std::array<Bitboard, 4096> attacks = { };
//     std::array<Bitboard, 4096> used_attacks = { };
//     auto attack_mask   = GetAttack<Piece>::MaskTable[square];
//     auto relevant_bits = GetAttack<Piece>::MaskBitCount[square];
//     int occupancy_idx = 1 << relevant_bits;

//     for (int idx = 0; idx < occupancy_idx; idx++) {
//         occupancies[idx] = GetOccupancy(idx, attack_mask);
//         attacks[idx] = SliderAttacks<Piece>::On(square, occupancies[idx]);
//     }

//     for (int random_count = 0; random_count < 10000000; random_count++) {
//         Bitboard magic = rng64() & rng64() & rng64();
//         if (Utils::BitCount((attack_mask * magic) ^ 56) < 6) continue;
//         used_attacks.fill(0x00);
//         int idx, fail;
//         for (idx = 0, fail = 0; !fail && idx < occupancy_idx; idx++ ) {
//             Bitboard magic_idx = (occupancies[idx] * magic) >> FixedShift;//relevant_bits);
//             if (used_attacks[magic_idx] == 0ULL)
//                 used_attacks[magic_idx] = attacks[idx];
//             else if (used_attacks[magic_idx] != attacks[idx]) fail = 1;
//         } if (!fail) return magic;
//     }
//     std::cout << "magic failed\n";
//     return Bitboard(0);
// }

// template <EnumPiece Piece>
// auto GenerateAndPrintMagicNumbers() noexcept {
//     constexpr auto PieceName = Piece == Bishops ? "<Bishops>" : "<Rooks>";
//     std::cout << "template<>\n"
//         << "constexpr inline std::array<Bitboard, 64> Magics" << PieceName << "{\n";
//     for (EnumSquare square = a1; square <= h8; ++square)
//         std::cout << std::hex << "0x" << FindMagicNumber<Piece>(square) << "ULL,\n";
//     std::cout << "};\n\n";
// }

// template <EnumPiece Piece>
// struct SliderAttacks final { };

// template <>
// struct SliderAttacks<Bishops> final {
// public:
//     [[nodiscard]] static constexpr auto On(EnumSquare square, Bitboard occupancy) noexcept {
//         Bitboard attack = 0ULL, b = 0ULL, o = occupancy;
//            int tr = square / 8, tf = square % 8; // 2D Square Index
//            #define SET_SQUARE { b=0; b|=EnumSquare(f+r*8); attack|=b; if (b&o) break; }
//            for (int r = tr+1, f = tf+1; r <= 7 && f <= 7; r++,f++) SET_SQUARE // NE
//            for (int r = tr+1, f = tf-1; r <= 7 && f >= 0; r++,f--) SET_SQUARE // NW
//            for (int r = tr-1, f = tf+1; r >= 0 && f <= 7; r--,f++) SET_SQUARE // SE
//            for (int r = tr-1, f = tf-1; r >= 0 && f >= 0; r--,f--) SET_SQUARE // SW
//            #undef  SET_SQUARE
//            return attack;
//     }
// };

// template <>
// struct SliderAttacks<Rooks> final {
// public:
//     [[nodiscard]] static constexpr auto On(EnumSquare square, Bitboard occupancy) noexcept {
//         Bitboard a = 0ULL, rk = 0ULL, o = occupancy;
//         int tr = square / 8, tf = square % 8;
//         #define SET_SQUARE { rk=0; rk|=EnumSquare(f+r*8); a|=rk; if (rk&o) break; }
//         for (int r = tr+1, f = tf;   r <= 7; r++) SET_SQUARE // N
//         for (int r = tr-1, f = tf;   r >= 0; r--) SET_SQUARE // S
//         for (int r = tr,   f = tf+1; f <= 7; f++) SET_SQUARE // E
//         for (int r = tr,   f = tf-1; f >= 0; f--) SET_SQUARE // W
//         #undef  SET_SQUARE
//         return a;
//     }
// };
