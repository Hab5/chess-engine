#pragma once

#include "ChessEngine.hpp"

#include <array>
#include <iostream>


namespace Generator {

using MaskTable = std::array<std::uint64_t, 64>;

template <EnumColor color, EnumPiece type>
class AttackMask final {
public:
    static constexpr auto Get() noexcept {
        switch(type) {
        case EnumPiece::Pawns:   return AttackMask::Pawn();
        case EnumPiece::Knights: return AttackMask::Knight();
        case EnumPiece::Bishops: return AttackMask::Bishop();
        case EnumPiece::Rooks:   return AttackMask::Rook();
        case EnumPiece::Queens:  return AttackMask::Queen();
        case EnumPiece::King:    return AttackMask::King();
        }
    }

private:

    [[nodiscard]] static constexpr std::array<std::uint64_t, 64> Pawn() noexcept {
        std::array<std::uint64_t, 64> masks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            std::uint64_t pawn = 0ULL | (1ULL << square);
            switch (color) {
            case White:
                if ((pawn << 9) & ~File_A) masks[square] |= (pawn << 9); // NW
                if ((pawn << 7) & ~File_H) masks[square] |= (pawn << 7); // NE
                break;
            case Black:
                if ((pawn >> 7) & ~File_A) masks[square] |= (pawn >> 7); // SE
                if ((pawn >> 9) & ~File_H) masks[square] |= (pawn >> 9); // SW
                break;
            }
        } return masks;
    }

    [[nodiscard]] static constexpr MaskTable Knight() noexcept {
        std::array<std::uint64_t, 64> masks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            std::uint64_t knight = 0ULL | (1ULL << square);
            if ((knight << 17) & ~File_A)            masks[square] |= (knight << 17); // NNE
            if ((knight << 15) & ~File_H)            masks[square] |= (knight << 15); // NNW
            if ((knight << 10) & ~(File_A | File_B)) masks[square] |= (knight << 10); // NE
            if ((knight << 6 ) & ~(File_G | File_H)) masks[square] |= (knight << 6 ); // NW

            if ((knight >> 17) & ~File_H)            masks[square] |= (knight >> 17); // SSW
            if ((knight >> 15) & ~File_A)            masks[square] |= (knight >> 15); // SSE
            if ((knight >> 10) & ~(File_G | File_H)) masks[square] |= (knight >> 10); // SW
            if ((knight >> 6 ) & ~(File_A | File_B)) masks[square] |= (knight >> 6 ); // SE
        } return masks;
    }

    [[nodiscard]] static constexpr MaskTable Bishop() noexcept { // Magic Bitboards
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

    [[nodiscard]] static constexpr MaskTable Rook() noexcept { // Magic Bitboards
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

    [[nodiscard]] static constexpr MaskTable Queen() noexcept { return {0ULL}; }

    [[nodiscard]] static constexpr MaskTable King() noexcept {
        std::array<std::uint64_t, 64> masks { };
        for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++) {
            std::uint64_t king = 0ULL | (1ULL << square);
            if  (king << 8)            masks[square] |= (king << 8); // N
            if ((king << 9) & ~File_A) masks[square] |= (king << 9); // NE
            if ((king << 7) & ~File_H) masks[square] |= (king << 7); // NW

            if ((king << 1) & ~File_A) masks[square] |= (king << 1); // E
            if ((king >> 1) & ~File_H) masks[square] |= (king >> 1); // W

            if ((king >> 9) & ~File_H) masks[square] |= (king >> 9); // SW
            if ((king >> 7) & ~File_A) masks[square] |= (king >> 7); // SE
            if  (king >> 8)            masks[square] |= (king >> 8); // S
        } return masks;
    }

     AttackMask() = delete;
    ~AttackMask() = delete;
};

}






template <EnumColor Color, EnumPiece Piece>
struct AttackMask final {
public:
    [[nodiscard]] static constexpr auto On(std::size_t square) noexcept {
        return MaskTable[square];
    };

private:
    static constexpr auto MaskTable = Generator::AttackMask<Color, Piece>::Get();

     AttackMask() = delete;
    ~AttackMask() = delete;
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
