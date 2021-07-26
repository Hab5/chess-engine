#pragma once

#include "ChessEngine.hpp"
#include "Utils.hpp"
#include "Magics.hpp"

#include <array>
#include <iostream>
#include <sys/types.h>

namespace Generator {

template <auto... EnumColor_EnumPiece>
class Attacks final { };

template <EnumColor Color, EnumPiece Piece>
class Attacks<Color, Piece> final {
public:
    [[nodiscard]] static constexpr auto Get() noexcept {
        static_assert(Piece == Pawns,
            "Use AttackMask<Piece> for anything other than `Pawns`\n");
        return Attacks::Pawn();
    }

private:
    [[nodiscard]] static constexpr auto Pawn() noexcept {
        std::array<std::uint64_t, 64> attacks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            std::uint64_t pawn = 0ULL | (1ULL << square);
            switch (Color) {
            case White:
                if ((pawn << 9) & ~File_A) attacks[square] |= (pawn << 9); // NW
                if ((pawn << 7) & ~File_H) attacks[square] |= (pawn << 7); // NE
                break;
            case Black:
                if ((pawn >> 7) & ~File_A) attacks[square] |= (pawn >> 7); // SE
                if ((pawn >> 9) & ~File_H) attacks[square] |= (pawn >> 9); // SW
                break;
            }
        } return attacks;
    }

     Attacks() = delete;
    ~Attacks() = delete;
};
    
template <EnumPiece Piece>
class Attacks<Piece> final {
public:
    [[nodiscard]] static constexpr auto Get() noexcept {
        switch(Piece) {
        case EnumPiece::Knights: return Attacks::Knight();
        // case EnumPiece::Bishops: return Attacks::Bishop();
        // case EnumPiece::Rooks:   return Attacks::Rook();
        case EnumPiece::Queens:  return Attacks::Queen();
        case EnumPiece::King:    return Attacks::King();
        }
    }

private:
    [[nodiscard]] static constexpr auto Knight() noexcept {
        std::array<std::uint64_t, 64> attacks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            std::uint64_t knight = 0ULL | (1ULL << square);
            if ((knight << 17) & ~File_A)            attacks[square] |= (knight << 17); // NNE
            if ((knight << 15) & ~File_H)            attacks[square] |= (knight << 15); // NNW
            if ((knight << 10) & ~(File_A | File_B)) attacks[square] |= (knight << 10); // NE
            if ((knight << 6 ) & ~(File_G | File_H)) attacks[square] |= (knight << 6 ); // NW

            if ((knight >> 17) & ~File_H)            attacks[square] |= (knight >> 17); // SSW
            if ((knight >> 15) & ~File_A)            attacks[square] |= (knight >> 15); // SSE
            if ((knight >> 10) & ~(File_G | File_H)) attacks[square] |= (knight >> 10); // SW
            if ((knight >> 6 ) & ~(File_A | File_B)) attacks[square] |= (knight >> 6 ); // SE
        } return attacks;
    }

    [[nodiscard]] static constexpr auto Bishop() noexcept { // For Magic Bitboards
        std::array<std::uint64_t, 64> masks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            int target_rank = square / 8, target_file = square % 8; // 2D Square Index
            int r = target_rank+1, f = target_file+1;
            while (r <= 6 && f <= 6) masks[square] |= (1ULL << ((8*r++)+(f++))); // NE
            r = target_rank+1, f = target_file-1;
            while (r <= 6 && f >= 1) masks[square] |= (1ULL << ((8*r++)+(f--))); // SE
            r = target_rank-1, f = target_file+1;
            while (r >= 1 && f <= 6) masks[square] |= (1ULL << ((8*r--)+(f++))); // SE
            r = target_rank-1, f = target_file-1;
            while (r >= 1 && f >= 1) masks[square] |= (1ULL << ((8*r--)+(f--))); // SW
        } return masks;
    }

    [[nodiscard]] static constexpr auto Rook() noexcept { // For Magic Bitboards
        std::array<std::uint64_t, 64> masks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            int target_rank = square / 8, target_file = square % 8; // 2D Square Index
            int r = target_rank+1, f = target_file;
            while (r <= 6) masks[square] |= (1ULL << ((8*r++)+f)); // N
            r = target_rank-1, f = target_file;
            while (r >= 1) masks[square] |= (1ULL << ((8*r--)+f)); // S
            r = target_rank, f = target_file+1;
            while (f <= 6) masks[square] |= (1ULL << (8*r+(f++))); // E
            r = target_rank, f = target_file-1;
            while (f >= 1) masks[square] |= (1ULL << (8*r+(f--))); // W
        } return masks;
    }

    [[nodiscard]] static constexpr auto Queen() noexcept { return std::array<std::uint64_t, 64>{ }; }

    [[nodiscard]] static constexpr auto King() noexcept {
        std::array<std::uint64_t, 64> attacks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            std::uint64_t king = 0ULL | (1ULL << square);
            if  (king << 8)            attacks[square] |= (king << 8); // N
            if ((king << 9) & ~File_A) attacks[square] |= (king << 9); // NE
            if ((king << 7) & ~File_H) attacks[square] |= (king << 7); // NW

            if ((king << 1) & ~File_A) attacks[square] |= (king << 1); // E
            if ((king >> 1) & ~File_H) attacks[square] |= (king >> 1); // W

            if ((king >> 9) & ~File_H) attacks[square] |= (king >> 9); // SW
            if ((king >> 7) & ~File_A) attacks[square] |= (king >> 7); // SE
            if  (king >> 8)            attacks[square] |= (king >> 8); // S
        } return attacks;
    }

     Attacks() = delete;
    ~Attacks() = delete;
};

template <EnumPiece Piece>
class Slider final { };

template <>
class Slider<Rooks> final {
public:
    [[nodiscard]] static constexpr auto Masks() noexcept {
        std::array<std::uint64_t, 64> masks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            int target_rank = square / 8, target_file = square % 8; // 2D Square Index
            int r = target_rank+1, f = target_file;
            while (r <= 6) masks[square] |= (1ULL << ((8*r++)+f)); // N
            r = target_rank-1, f = target_file;
            while (r >= 1) masks[square] |= (1ULL << ((8*r--)+f)); // S
            r = target_rank, f = target_file+1;
            while (f <= 6) masks[square] |= (1ULL << (8*r+(f++))); // E
            r = target_rank, f = target_file-1;
            while (f >= 1) masks[square] |= (1ULL << (8*r+(f--))); // W
        } return masks;
    }

    [[nodiscard]] static constexpr auto MasksBitCount() noexcept {
        std::array<std::uint64_t, 64> masks = Masks();
        std::array<int, 64> masks_bitcount = { };
        for (int square = 0; square < 64; square++)
            masks_bitcount[square] = Utils::BitCount(masks[square]);
        return masks_bitcount;
    }

    [[nodiscard]] static constexpr auto Attacks() noexcept {
        auto on_the_fly = [](std::size_t square, std::uint64_t occupancy) constexpr {
            std::uint64_t attack = 0ULL, rook = 0ULL;
            int target_rank = square / 8, target_file = square % 8; // 2D Square Index
            int r = target_rank+1, f = target_file;
            while (r <= 7) { // N
                rook = (1ULL << ((8*r++)+f));
                attack |= rook; if (rook & occupancy) break;
            } r = target_rank-1, f = target_file;
            while (r >= 0) { // S
                rook = (1ULL << ((8*r--)+f));
                attack |= rook; if (rook & occupancy) break;
            } r = target_rank, f = target_file+1;
            while (f <= 7) { // E
                rook = (1ULL << (8*r+(f++)));
                attack |= rook; if (rook & occupancy) break;
            } r = target_rank, f = target_file-1;
            while (f >= 0) { // W
                rook = (1ULL << (8*r+(f--)));
                attack |= rook; if (rook & occupancy) break;
            }
            return attack;
        };

        auto get_occupancy = [](int index, std::uint64_t attack_mask) constexpr {
            std::uint64_t occupancy = 0ULL;
            const auto mask_population = Utils::BitCount(attack_mask);
            for (int count = 0; count < mask_population; count++) {
                auto square = Utils::IndexLS1B(attack_mask);
                if (attack_mask  & (1ULL << square)) // TODO: Make PopSquare() constexpr here
                    attack_mask ^= (1ULL << square);
                if (index & (1 << count))
                    occupancy |= (1ULL << square);
            }
            return occupancy;
        };


        std::array<std::uint64_t, 64> masks = Masks();
        std::array<int, 64> masks_bitcount = MasksBitCount();
        std::array<std::array<std::uint64_t, 4096>, 64> attacks { };
        for (int square = 0; square < 64; square++) {
            auto attack_mask = masks[square];
            auto relevant_bits = masks_bitcount[square];
            int occupancy_idx = 1 << relevant_bits;
            for (int idx = 0; idx < occupancy_idx; idx++) {
                auto occupancy = get_occupancy(idx, attack_mask);
                int magic_index = (occupancy * Magics<Rooks>[square]) >> (64-relevant_bits);
                attacks[square][magic_index] = on_the_fly(square, occupancy);
            }
        } return attacks;
    }

private:
     Slider() = delete;
    ~Slider() = delete;
};

template <>
class Slider<Bishops> final {
public:
    [[nodiscard]] static constexpr auto Masks() noexcept {
        std::array<std::uint64_t, 64> masks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            int target_rank = square / 8, target_file = square % 8; // 2D Square Index
            int r = target_rank+1, f = target_file+1;
            while (r <= 6 && f <= 6) masks[square] |= (1ULL << ((8*r++)+(f++))); // NE
            r = target_rank+1, f = target_file-1;
            while (r <= 6 && f >= 1) masks[square] |= (1ULL << ((8*r++)+(f--))); // NW
            r = target_rank-1, f = target_file+1;
            while (r >= 1 && f <= 6) masks[square] |= (1ULL << ((8*r--)+(f++))); // SE
            r = target_rank-1, f = target_file-1;
            while (r >= 1 && f >= 1) masks[square] |= (1ULL << ((8*r--)+(f--))); // SW
        } return masks;
    }

    [[nodiscard]] static constexpr auto MasksBitCount() noexcept {
        std::array<std::uint64_t, 64> masks = Masks();
        std::array<int, 64> masks_bitcount = { };
        for (int square = 0; square < 64; square++)
            masks_bitcount[square] = Utils::BitCount(masks[square]);
        return masks_bitcount;
    }

    [[nodiscard]] static constexpr auto Attacks() noexcept {
        auto on_the_fly = [](std::size_t square, std::uint64_t occupancy) constexpr {
            std::uint64_t attacks = 0ULL, bishop = 0ULL;
            int target_rank = square / 8, target_file = square % 8; // 2D Square Index
            int r = target_rank+1, f = target_file+1;
            while (r <= 7 && f <= 7) { // NE
                bishop = (1ULL << ((8*r++)+(f++)));
                attacks |= bishop; if (bishop & occupancy) break;
            } r = target_rank+1, f = target_file-1;
            while (r <= 7 && f >= 0) { // NW
                bishop = (1ULL << ((8*r++)+(f--)));
                attacks |= bishop; if (bishop & occupancy) break;
            } r = target_rank-1, f = target_file+1;
            while (r >= 0 && f <= 7) { // SE
                bishop = (1ULL << ((8*r--)+(f++)));
                attacks |= bishop; if (bishop & occupancy) break;
            } r = target_rank-1, f = target_file-1;
            while (r >= 0 && f >= 0) { // SW
                bishop = (1ULL << ((8*r--)+(f--)));
                attacks |= bishop; if (bishop & occupancy) break;
            }
            return attacks;

        };

        auto get_occupancy = [](int index, std::uint64_t attack_mask) constexpr {
            std::uint64_t occupancy = 0ULL;
            const auto mask_population = Utils::BitCount(attack_mask);
            for (int count = 0; count < mask_population; count++) {
                auto square = Utils::IndexLS1B(attack_mask);
                if (attack_mask  & (1ULL << square)) // TODO: Make PopSquare() constexpr here
                    attack_mask ^= (1ULL << square);
                if (index & (1 << count))
                    occupancy |= (1ULL << square);
            }
            return occupancy;
        };

        std::array<std::uint64_t, 64> masks = Masks();
        std::array<int, 64> masks_bitcount = MasksBitCount();
        std::array<std::array<std::uint64_t, 4096>, 64> attacks { }; // ERROR
        for (int square = 0; square < 64; square++) {
            auto attack_mask = masks[square];
            auto relevant_bits = masks_bitcount[square];
            int occupancy_idx = 1 << relevant_bits;
            for (int idx = 0; idx < occupancy_idx; idx++) {
                auto occupancy = get_occupancy(idx, attack_mask);
                int magic_index = (occupancy * Magics<Bishops>[square]) >> (64-relevant_bits);
                attacks[square][magic_index] = on_the_fly(square, occupancy);
            }
        } return attacks;
    }

private:
     Slider() = delete;
    ~Slider() = delete;
};

}







template <auto... Args>
struct Attack final { }; // All pieces

template <EnumColor Color, EnumPiece Pawn>
struct Attack<Color, Pawn> final { // Pawns
public:
    [[nodiscard]] static constexpr auto On(std::size_t square) noexcept {
        return AttackTable[square];
    };

private:
    static constexpr auto AttackTable = Generator::Attacks<Color, Pawn>::Get();

     Attack() = delete;
    ~Attack() = delete;
};

template <EnumPiece Piece>
struct Attack<Piece> final { // Knight / King
public:
    [[nodiscard]] static constexpr auto On(std::size_t square) noexcept {
        return AttackTable[square];
    };

private:
    static constexpr auto AttackTable = Generator::Attacks<Piece>::Get();

     Attack() = delete;
    ~Attack() = delete;
};

template <>
struct Attack<Bishops> final { // Bishops
public:
    [[nodiscard]] static constexpr auto On(int square, std::uint64_t occupancy) noexcept {
        occupancy &= MaskTable[square];
        occupancy *= Magics<Bishops>[square];
        occupancy >>= 64 - MaskBitCount[square];
        return AttackTable[square][occupancy];
    }
    static constexpr auto AttackTable    = Generator::Slider<Bishops>::Attacks();
    static constexpr auto MaskTable      = Generator::Slider<Bishops>::Masks();
    static constexpr auto MaskBitCount   = Generator::Slider<Bishops>::MasksBitCount();
private:
};

template <>
struct Attack<Rooks> final { // Rooks
public:
    [[nodiscard]] static constexpr auto On(int square, std::uint64_t occupancy) noexcept {
        occupancy &= MaskTable[square];
        occupancy *= Magics<Rooks>[square];
        occupancy >>= 64 - MaskBitCount[square];
        return AttackTable[square][occupancy];
    }
    static constexpr auto AttackTable    = Generator::Slider<Rooks>::Attacks();
    static constexpr auto MaskTable      = Generator::Slider<Rooks>::Masks();
    static constexpr auto MaskBitCount   = Generator::Slider<Rooks>::MasksBitCount();
private:
};

























template <EnumPiece Piece>
struct SliderAttacks final { };

template <>
struct SliderAttacks<Bishops> final {
public:
    [[nodiscard]] static constexpr auto On(std::size_t square, std::uint64_t blockers) noexcept {
        std::uint64_t masks = 0ULL, mask = 0ULL;
        int target_rank = square / 8, target_file = square % 8; // 2D Square Index
        int r = target_rank+1, f = target_file+1;
        while (r <= 7 && f <= 7) { // NE
            mask = (1ULL << ((8*r++)+(f++)));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank+1, f = target_file-1;
        while (r <= 7 && f >= 0) { // NE
            mask = (1ULL << ((8*r++)+(f--)));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank-1, f = target_file+1;
        while (r >= 0 && f <= 7) { // SE
            mask = (1ULL << ((8*r--)+(f++)));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank-1, f = target_file-1;
        while (r >= 0 && f >= 0) { // SE
            mask = (1ULL << ((8*r--)+(f--)));
            masks |= mask; if (mask & blockers) break;
        }
        return masks;
    }
};

template <>
struct SliderAttacks<Rooks> final {
public:
    [[nodiscard]] static constexpr auto On(std::size_t square, std::uint64_t blockers) noexcept {
        std::uint64_t masks = 0ULL, mask = 0ULL;
        int target_rank = square / 8, target_file = square % 8; // 2D Square Index
        int r = target_rank+1, f = target_file;
        while (r <= 7) { // N
            mask = (1ULL << ((8*r++)+f));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank-1, f = target_file;
        while (r >= 0) { // S
            mask = (1ULL << ((8*r--)+f));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank, f = target_file+1;
        while (f <= 7) { // E
            mask = (1ULL << (8*r+(f++)));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank, f = target_file-1;
        while (f >= 0) { // W
            mask = (1ULL << (8*r+(f--)));
            masks |= mask; if (mask & blockers) break;
        }
        return masks;
    }
};
