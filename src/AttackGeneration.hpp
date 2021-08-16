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

    [[nodiscard]] static _constexpr auto MaskTablePopCount() noexcept {
        std::array<Bitboard, 64> masks = MaskTable();
        std::array<int, 64> masks_bitcount = { };
        for (EnumSquare square = a1; square <= h8; ++square)
            masks_bitcount[square] = Utils::PopCount(masks[square]);
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
            const auto population_count = Utils::PopCount(attack_mask);
            for (int count = 0; count < population_count; count++) {
                auto square = Utils::PopLS1B(attack_mask);
                if (index & (1 << count))
                    occupancy |= (1ULL << square);
            } return occupancy;
        };

        std::array<Bitboard, 64> masks      = MaskTable();
        std::array<int, 64> masks_population_count  = MaskTablePopCount();
        std::array<std::array<Bitboard, 512>, 64> attacks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            auto attack_mask = masks[square];
            auto relevant_bits = masks_population_count[square];
            int permutations_count = 1 << relevant_bits;
            for (int index = 0; index < permutations_count; index++) {
                auto occupancy  = get_occupancy(index, attack_mask);
                int magic_index = (occupancy * _MagicNumbers[square]) >> (64-9);
                attacks[square][magic_index] = get_attack(square, occupancy);
            }
        } return attacks;
    }

    [[nodiscard]] static _constexpr auto MagicNumbers() noexcept { return _MagicNumbers; }

    [[nodiscard]] static _constexpr auto Magics_AOS() noexcept {
        struct Magic {
            std::array<Bitboard, 512>  Attack;
            Bitboard                   Mask;
            std::uint64_t              Number;
        };

        auto attacks = Generator::Attacks<Bishops>::AttackTable();
        auto masks   = Generator::Attacks<Bishops>::MaskTable();
        auto numbers = Generator::Attacks<Bishops>::MagicNumbers();

        std::array<Magic, 64> Magics;
        for (EnumSquare square = a1; square <= h8; ++square) {
            Magics[square] = Magic {
                .Attack = attacks[square],
                .Mask   = masks  [square],
                .Number = numbers[square],
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

    [[nodiscard]] static _constexpr auto MaskTablePopCount() noexcept {
        std::array<Bitboard, 64> masks = MaskTable();
        std::array<int, 64> masks_population_count = { };
        for (EnumSquare square = a1; square <= h8; ++square)
            masks_population_count[square] = Utils::PopCount(masks[square]);
        return masks_population_count;
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
            const auto population_count = Utils::PopCount(attack_mask);
            for (int count = 0; count < population_count; count++) {
                auto square = Utils::PopLS1B(attack_mask);
                if (index & (1 << count))
                    occupancy |= (1ULL << square);
            } return occupancy;
        };

        std::array<Bitboard, 64> masks = MaskTable();
        std::array<int, 64> masks_population_count  = MaskTablePopCount();
        std::array<std::array<Bitboard, 4096>, 64> attacks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            auto attack_mask = masks[square];
            auto population_count = masks_population_count[square];
            int permutations_count = 1 << population_count;
            for (int idx = 0; idx < permutations_count; idx++) {
                auto occupancy = get_occupancy(idx, attack_mask);
                int magic_index = (occupancy * _MagicNumbers[square]) >> (64-12);
                attacks[square][magic_index] = get_attack(square, occupancy);
            }
        } return attacks;
    }

    [[nodiscard]] static _constexpr auto MagicNumbers() noexcept { return _MagicNumbers; }

    [[nodiscard]] static _constexpr auto Magics_AOS() {
        struct Magic {
            std::array<Bitboard, 4096> Attack;
            Bitboard                   Mask;
            std::uint64_t              Number;
        };

        auto attacks = Generator::Attacks<Rooks>::AttackTable();
        auto masks   = Generator::Attacks<Rooks>::MaskTable();
        auto numbers = Generator::Attacks<Rooks>::MagicNumbers();

        std::array<Magic, 64> Magics;
        for (EnumSquare square = a1; square <= h8; ++square) {
            Magics[square] = Magic {
                .Attack = attacks[square],
                .Mask   = masks  [square],
                .Number = numbers[square],
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

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// FANCY MAGIC BITBOARDS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////// BISHOPS ///////////////////////////////////////////

// template <>
// class Attacks<Bishops> final {
// public:
//      [[nodiscard]] static _constexpr auto MaskTable() noexcept {
//         std::array<Bitboard, 64> masks { };
//         for (EnumSquare square = a1; square <= h8; ++square) {
//             int tr = square / 8, tf = square % 8; // 2D Square Index
//             #define SET_SQUARE masks[square] |= EnumSquare(f+r*8);
//             for (int r = tr+1, f = tf+1; r <= 6 && f <= 6; r++,f++) SET_SQUARE // NE
//             for (int r = tr+1, f = tf-1; r <= 6 && f >= 1; r++,f--) SET_SQUARE // NW
//             for (int r = tr-1, f = tf+1; r >= 1 && f <= 6; r--,f++) SET_SQUARE // SE
//             for (int r = tr-1, f = tf-1; r >= 1 && f >= 1; r--,f--) SET_SQUARE // SW
//             #undef  SET_SQUARE
//         } return masks;
//     }

//     [[nodiscard]] static _constexpr auto MaskTableBitCount() noexcept {
//         std::array<Bitboard, 64> masks = MaskTable();
//         std::array<int, 64> masks_bitcount = { };
//         for (EnumSquare square = a1; square <= h8; ++square)
//             masks_bitcount[square] = Utils::BitCount(masks[square]);
//         return masks_bitcount;
//     }

//     [[nodiscard]] static _constexpr auto AttackTable() noexcept {
//          auto get_attack = [](EnumSquare square, Bitboard occupancy) constexpr {
//             Bitboard attack = 0ULL, b = 0ULL, o = occupancy;
//             int tr = square / 8, tf = square % 8; // 2D Square Index
//             #define SET_SQUARE { b=0; b|=EnumSquare(f+r*8); attack|=b; if (b&o) break; }
//             for (int r = tr+1, f = tf+1; r <= 7 && f <= 7; r++,f++) SET_SQUARE // NE
//             for (int r = tr+1, f = tf-1; r <= 7 && f >= 0; r++,f--) SET_SQUARE // NW
//             for (int r = tr-1, f = tf+1; r >= 0 && f <= 7; r--,f++) SET_SQUARE // SE
//             for (int r = tr-1, f = tf-1; r >= 0 && f >= 0; r--,f--) SET_SQUARE // SW
//             #undef  SET_SQUARE
//             return attack;
//         };

//         auto get_occupancy = [](int index, Bitboard attack_mask) constexpr {
//             Bitboard occupancy = 0ULL;
//             const auto mask_population = Utils::BitCount(attack_mask);
//             for (int count = 0; count < mask_population; count++) {
//                 auto square = Utils::PopLS1B(attack_mask);
//                 if (index & (1 << count))
//                     occupancy |= (1ULL << square);
//             } return occupancy;
//         };

//         std::array<Bitboard, 64> masks          = MaskTable();
//         std::array<int,      64> masks_bitcount = MaskTableBitCount();

//         for (EnumSquare square = a1; square <= h8; ++square) {

//             auto attack_mask   = masks[square];
//             auto relevant_bits = masks_bitcount[square];
//             int  permutations  = 1 << relevant_bits;

//             for (int index = 0; index < permutations; index++) {
//                 auto occupancy = get_occupancy(index, attack_mask);
//                 int magic_index = (occupancy * Numbers[square]) >> Shifts[square];
//                 *(Pointers[square] + magic_index) = get_attack(square, occupancy);
//             }
//         }
//     }

//     [[nodiscard]] static _constexpr auto FancyMagics_SOA() noexcept {
//         struct Magic {
//             // std::array<std::uint64_t, 5248> Table;
//             std::array<Bitboard*,       64> Attack;
//             std::array<Bitboard,        64> Mask;
//             std::array<std::uint64_t,   64> Number;
//             std::array<std::int32_t,    64> Shift;
//         };

//         Generator::Attacks<Bishops>::AttackTable();

//         return Magic {
//             // .Table   = Table,
//             .Attack  = Pointers,
//             .Mask    = MaskTable(),
//             .Number  = Numbers,
//             .Shift   = Shifts,
//         };
//     }

//     [[nodiscard]] static _constexpr auto FancyMagics_AOS() noexcept {
//         struct Magic {
//             Bitboard*     Attack;
//             Bitboard      Mask;
//             std::uint64_t Number;
//             int           Shift;
//         };

//         Generator::Attacks<Bishops>::AttackTable();
//         std::array<Magic, 64> Magics { };
//         for (auto square = a1; square <= h8; ++square) {
//             Magics[square] = Magic {
//                 .Attack  = Pointers[square],
//                 .Mask    = MaskTable()[square],
//                 .Number  = Numbers[square],
//                 .Shift   = Shifts[square],
//             };
//         }
//         return Magics;
//     }


// private:

//     static inline std::array<std::uint64_t, 5248> Table { };
//     static inline std::array<std::uint64_t*,  64> Pointers = {
//     &Table[0] + 4992, &Table[0] + 2624, &Table[0] + 256,  &Table[0] + 896,
//     &Table[0] + 1280, &Table[0] + 1664, &Table[0] + 4800, &Table[0] + 5120,
//     &Table[0] + 2560, &Table[0] + 2656, &Table[0] + 288,  &Table[0] + 928,
//     &Table[0] + 1312, &Table[0] + 1696, &Table[0] + 4832, &Table[0] + 4928,
//     &Table[0] + 0,    &Table[0] + 128,  &Table[0] + 320,  &Table[0] + 960,
//     &Table[0] + 1344, &Table[0] + 1728, &Table[0] + 2304, &Table[0] + 2432,
//     &Table[0] + 32,   &Table[0] + 160,  &Table[0] + 448,  &Table[0] + 2752,
//     &Table[0] + 3776, &Table[0] + 1856, &Table[0] + 2336, &Table[0] + 2464,
//     &Table[0] + 64,   &Table[0] + 192,  &Table[0] + 576,  &Table[0] + 3264,
//     &Table[0] + 4288, &Table[0] + 1984, &Table[0] + 2368, &Table[0] + 2496,
//     &Table[0] + 96,   &Table[0] + 224,  &Table[0] + 704,  &Table[0] + 1088,
//     &Table[0] + 1472, &Table[0] + 2112, &Table[0] + 2400, &Table[0] + 2528,
//     &Table[0] + 2592, &Table[0] + 2688, &Table[0] + 832,  &Table[0] + 1216,
//     &Table[0] + 1600, &Table[0] + 2240, &Table[0] + 4864, &Table[0] + 4960,
//     &Table[0] + 5056, &Table[0] + 2720, &Table[0] + 864,  &Table[0] + 1248,
//     &Table[0] + 1632, &Table[0] + 2272, &Table[0] + 4896, &Table[0] + 5184 };

//     const static inline std::array<std::uint64_t, 64> Numbers {
//     0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
//     0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
//     0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
//     0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
//     0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
//     0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
//     0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
//     0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
//     0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
//     0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
//     0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
//     0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
//     0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
//     0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
//     0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
//     0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL };

//     const static inline std::array<int, 64> Shifts {
//     58, 59, 59, 59, 59, 59, 59, 58,
// 	59, 59, 59, 59, 59, 59, 59, 59,
// 	59, 59, 57, 57, 57, 57, 59, 59,
// 	59, 59, 57, 55, 55, 57, 59, 59,
// 	59, 59, 57, 55, 55, 57, 59, 59,
// 	59, 59, 57, 57, 57, 57, 59, 59,
// 	59, 59, 59, 59, 59, 59, 59, 59,
// 	58, 59, 59, 59, 59, 59, 59, 58 };

//      Attacks() = delete;
//     ~Attacks() = delete;
// };

////////////////////////////////////////// ROOKS //////////////////////////////////////////////

// template <>
// class Attacks<Rooks> final {
// public:
//      [[nodiscard]] static _constexpr auto MaskTable() noexcept {
//         std::array<Bitboard, 64> masks { };
//         for (EnumSquare square = a1; square <= h8; ++square) {
//             int tr = square / 8, tf = square % 8; // 2D Square Index
//             #define SET_SQUARE masks[square] |= EnumSquare(f+r*8);
//             for (int r = tr+1, f = tf;   r <= 6; r++) SET_SQUARE // N
//             for (int r = tr-1, f = tf;   r >= 1; r--) SET_SQUARE // S
//             for (int r = tr,   f = tf+1; f <= 6; f++) SET_SQUARE // E
//             for (int r = tr,   f = tf-1; f >= 1; f--) SET_SQUARE // W
//             #undef  SET_SQUARE
//         } return masks;
//     }

//     [[nodiscard]] static _constexpr auto MaskTableBitCount() noexcept {
//         std::array<Bitboard, 64> masks = MaskTable();
//         std::array<int, 64> masks_bitcount = { };
//         for (EnumSquare square = a1; square <= h8; ++square)
//             masks_bitcount[square] = Utils::BitCount(masks[square]);
//         return masks_bitcount;
//     }

//     [[nodiscard]] static _constexpr auto AttackTable() noexcept {
//         auto get_attack = [](EnumSquare square, Bitboard occupancy) constexpr {
//             Bitboard a = 0ULL, rk = 0ULL, o = occupancy;
//             int tr = square / 8, tf = square % 8;
//             #define SET_SQUARE { rk=0; rk|=EnumSquare(f+r*8); a|=rk; if (rk&o) break; }
//             for (int r = tr+1, f = tf;   r <= 7; r++) SET_SQUARE // N
//             for (int r = tr-1, f = tf;   r >= 0; r--) SET_SQUARE // S
//             for (int r = tr,   f = tf+1; f <= 7; f++) SET_SQUARE // E
//             for (int r = tr,   f = tf-1; f >= 0; f--) SET_SQUARE // W
//             #undef  SET_SQUARE
//             return a;
//         };

//         auto get_occupancy = [](int index, Bitboard attack_mask) constexpr {
//             Bitboard occupancy = 0ULL;
//             const auto mask_population = Utils::BitCount(attack_mask);
//             for (int count = 0; count < mask_population; count++) {
//                 auto square = Utils::PopLS1B(attack_mask);
//                 if (index & (1 << count))
//                     occupancy |= (1ULL << square);
//             } return occupancy;
//         };

//         std::array<Bitboard, 64> masks          = MaskTable();
//         std::array<int,      64> masks_bitcount = MaskTableBitCount();

//         for (EnumSquare square = a1; square <= h8; ++square) {

//             auto attack_mask   = masks[square];
//             auto relevant_bits = masks_bitcount[square];
//             int  permutations  = 1 << relevant_bits;

//             for (int index = 0; index < permutations; index++) {
//                 auto occupancy = get_occupancy(index, attack_mask);
//                 int magic_index = (occupancy * Numbers[square]) >> Shifts[square];
//                 *(Pointers[square] + magic_index) = get_attack(square, occupancy);
//             }
//         }
//     }

//     [[nodiscard]] static _constexpr auto FancyMagics_SOA() noexcept {
//         struct Magic {
//             // std::array<std::uint64_t, 102400> Table;
//             std::array<Bitboard*,     64> Attack;
//             std::array<Bitboard,      64> Mask;
//             std::array<std::uint64_t, 64> Number;
//             std::array<std::int32_t,  64> Shift;
//         };

//         Generator::Attacks<Rooks>::AttackTable();

//         return Magic {
//             // .Table   = RTable,
//             .Attack  = Pointers,
//             .Mask    = MaskTable(),
//             .Number  = Numbers,
//             .Shift   = Shifts,
//         };
//     }

//     [[nodiscard]] static _constexpr auto FancyMagics_AOS() noexcept {
//         struct Magic {
//             Bitboard*     Attack;
//             Bitboard      Mask;
//             std::uint64_t Number;
//             int           Shift;
//         };

//         Generator::Attacks<Rooks>::AttackTable();
//         std::array<Magic, 64> Magics { };
//         for (auto square = a1; square <= h8; ++square) {
//             Magics[square] = Magic {
//                 .Attack  = Pointers[square],
//                 .Mask    = MaskTable()[square],
//                 .Number  = Numbers[square],
//                 .Shift   = Shifts[square],
//             };
//         }
//         return Magics;
//     }


// private:

//     static inline std::array<std::uint64_t, 102400> Table { };
//     static inline std::array<std::uint64_t*,    64> Pointers = {
//     &Table[0] + 86016, &Table[0] + 73728, &Table[0] + 36864, &Table[0] + 43008,
//     &Table[0] + 47104, &Table[0] + 51200, &Table[0] + 77824, &Table[0] + 94208,
//     &Table[0] + 69632, &Table[0] + 32768, &Table[0] + 38912, &Table[0] + 10240,
//     &Table[0] + 14336, &Table[0] + 53248, &Table[0] + 57344, &Table[0] + 81920,
//     &Table[0] + 24576, &Table[0] + 33792, &Table[0] + 6144 , &Table[0] + 11264,
//     &Table[0] + 15360, &Table[0] + 18432, &Table[0] + 58368, &Table[0] + 61440,
//     &Table[0] + 26624, &Table[0] + 4096 , &Table[0] + 7168 , &Table[0] + 0    ,
//     &Table[0] + 2048 , &Table[0] + 19456, &Table[0] + 22528, &Table[0] + 63488,
//     &Table[0] + 28672, &Table[0] + 5120 , &Table[0] + 8192 , &Table[0] + 1024 ,
//     &Table[0] + 3072 , &Table[0] + 20480, &Table[0] + 23552, &Table[0] + 65536,
//     &Table[0] + 30720, &Table[0] + 34816, &Table[0] + 9216 , &Table[0] + 12288,
//     &Table[0] + 16384, &Table[0] + 21504, &Table[0] + 59392, &Table[0] + 67584,
//     &Table[0] + 71680, &Table[0] + 35840, &Table[0] + 39936, &Table[0] + 13312,
//     &Table[0] + 17408, &Table[0] + 54272, &Table[0] + 60416, &Table[0] + 83968,
//     &Table[0] + 90112, &Table[0] + 75776, &Table[0] + 40960, &Table[0] + 45056,
//     &Table[0] + 49152, &Table[0] + 55296, &Table[0] + 79872, &Table[0] + 98304 };

//     const static inline std::array<std::uint64_t, 64> Numbers {
//     0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
//     0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
//     0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
//     0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
//     0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
//     0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
//     0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
//     0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
//     0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
//     0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
//     0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
//     0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
//     0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
//     0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
//     0x00fffcddfced714aULL, 0x007ffcddfced714aULL, 0x003fffcdffd88096ULL, 0x0000040810002101ULL,
//     0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001fffaabfad1a2ULL };

//     const static inline std::array<int, 64> Shifts {
//     52, 53, 53, 53, 53, 53, 53, 52,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 53, 53, 53, 53, 53 };

//      Attacks() = delete;
//     ~Attacks() = delete;
// };
