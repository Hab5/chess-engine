#pragma once

#include "ChessEngine.hpp"
#include "Utils.hpp"
#include "Magics.hpp"

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
            if constexpr(Color == White) {
                attacks[square] |= ((pawn << 9) & ~File_A); // NW
                attacks[square] |= ((pawn << 7) & ~File_H); // NE
            } else if constexpr(Color == Black) {
                attacks[square] |= ((pawn >> 7) & ~File_A); // SE
                attacks[square] |= ((pawn >> 9) & ~File_H); // SW
            }
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
        if constexpr(Piece == Knights) return Attacks::Knight();
        if constexpr(Piece == King   ) return Attacks::King();
    }

private:
    [[nodiscard]] static constexpr auto Knight() noexcept {
        std::array<Bitboard, 64> attacks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            Bitboard knight = Utils::MakeSquare(square);
            attacks[square] |= ((knight << 17) & ~File_A);            // NNE
            attacks[square] |= ((knight << 15) & ~File_H);            // NNW
            attacks[square] |= ((knight << 10) & ~(File_A | File_B)); // NEE
            attacks[square] |= ((knight << 6 ) & ~(File_G | File_H)); // NWW
            attacks[square] |= ((knight >> 17) & ~File_H) ;           // SSW
            attacks[square] |= ((knight >> 15) & ~File_A);            // SSE
            attacks[square] |= ((knight >> 10) & ~(File_G | File_H)); // SWW
            attacks[square] |= ((knight >> 6 ) & ~(File_A | File_B)); // SEE
        } return attacks;
    }

    [[nodiscard]] static constexpr auto King() noexcept {
        std::array<Bitboard, 64> attacks { };
        for (EnumSquare square = a1; square <= h8; ++square) {
            Bitboard king = Utils::MakeSquare(square);
            attacks[square] |=  (king << 8);            // N
            attacks[square] |= ((king << 9) & ~File_A); // NE
            attacks[square] |= ((king << 7) & ~File_H); // NW
            attacks[square] |= ((king << 1) & ~File_A); // E
            attacks[square] |= ((king >> 1) & ~File_H); // W
            attacks[square] |= ((king >> 9) & ~File_H); // SW
            attacks[square] |= ((king >> 7) & ~File_A); // SE
            attacks[square] |=  (king >> 8);            // S
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

    [[nodiscard]] static constexpr auto MaskTableBitCount() noexcept {
        std::array<Bitboard, 64> masks = MaskTable();
        std::array<int, 64> masks_bitcount = { };
        for (EnumSquare square = a1; square <= h8; ++square)
            masks_bitcount[square] = Utils::BitCount(masks[square]);
        return masks_bitcount;
    }

    [[nodiscard]] static constexpr auto AttackTable() noexcept {
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
                int magic_index = (occupancy * Magics<Bishops>[square]) >> (64-relevant_bits);
                attacks[square][magic_index] = get_attack(square, occupancy);
            }
        } return attacks;
    }

private:
     Attacks() = delete;
    ~Attacks() = delete;
};

////////////////////////////////////////// ROOKS //////////////////////////////////////////////

template <>
class Attacks<Rooks> final {
public:
    [[nodiscard]] static constexpr auto MaskTable() noexcept {
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

    [[nodiscard]] static constexpr auto MaskTableBitCount() noexcept {
        std::array<Bitboard, 64> masks = MaskTable();
        std::array<int, 64> masks_bitcount = { };
        for (EnumSquare square = a1; square <= h8; ++square)
            masks_bitcount[square] = Utils::BitCount(masks[square]);
        return masks_bitcount;
    }

    [[nodiscard]] static constexpr auto AttackTable() noexcept {
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
                int magic_index = (occupancy * Magics<Rooks>[square]) >> (64-relevant_bits);
                attacks[square][magic_index] = get_attack(square, occupancy);
            }
        } return attacks;
    }

private:
     Attacks() = delete;
    ~Attacks() = delete;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
