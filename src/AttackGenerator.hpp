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
                int magic_idx = (occupancy * Magics<Bishops>[square]) >> (64-relevant_bits);
                attacks[square][magic_idx] = get_attack(square, occupancy);
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
                int magic_idx = (occupancy * Magics<Rooks>[square]) >> (64-relevant_bits);
                attacks[square][magic_idx] = get_attack(square, occupancy);
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
