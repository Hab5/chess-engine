#pragma once

#include "AttackGenerator.hpp"
#include "ChessEngine.hpp"
#include "Utils.hpp"
#include "Magics.hpp"

#include <array>
#include <iostream>
#include <sys/types.h>

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// ATTACK DISPATCHER //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

template <auto... Args>
struct GetAttack final { };

////////////////////////////////////////// PAWNS //////////////////////////////////////////////

template <EnumColor Color, EnumPiece Piece>
struct GetAttack<Color, Piece> final {
public:
    [[nodiscard]] static constexpr auto On(EnumSquare square) noexcept {
        static_assert(
            Piece == Pawns,
            "Use Attack<Piece> for anything other than `Pawns`\n"
        ); return AttackTable[square];
    }

private:
    static constexpr auto AttackTable = Generator::Attacks<Color, Pawns>::Get();

     GetAttack() = delete;
    ~GetAttack() = delete;
};

////////////////////////////////////// KNIGHTS / KING /////////////////////////////////////////

template <EnumPiece Piece>
struct GetAttack<Piece> final {
public:
    [[nodiscard]] static constexpr auto On(EnumSquare square) noexcept {
        return AttackTable[square];
    }

private:
    static constexpr auto AttackTable = Generator::Attacks<Piece>::Get();

     GetAttack() = delete;
    ~GetAttack() = delete;
};

///////////////////////////////////////// BISHOPS /////////////////////////////////////////////

template <>
struct GetAttack<Bishops> final {
public:
    [[nodiscard]] static constexpr auto On(EnumSquare square, Bitboard occupancy) noexcept {
        occupancy &= MaskTable[square];
        occupancy *= Magics<Bishops>[square];
        occupancy >>= 64 - MaskBitCount[square];
        return AttackTable[square][occupancy];
    }

private:
    static constexpr auto MaskBitCount = Generator::Attacks<Bishops>::MaskTableBitCount();
    static constexpr auto AttackTable  = Generator::Attacks<Bishops>::AttackTable();
    static constexpr auto MaskTable    = Generator::Attacks<Bishops>::MaskTable();

     GetAttack() = delete;
    ~GetAttack() = delete;
};

////////////////////////////////////////// ROOKS //////////////////////////////////////////////

template <>
struct GetAttack<Rooks> final {
public:
    [[nodiscard]] static constexpr auto On(EnumSquare square, Bitboard occupancy) noexcept {
        occupancy &= MaskTable[square];
        occupancy *= Magics<Rooks>[square];
        occupancy >>= 64 - MaskBitCount[square];
        return AttackTable[square][occupancy];
    }

private:
    static constexpr auto AttackTable  = Generator::Attacks<Rooks>::AttackTable();
    static constexpr auto MaskTable    = Generator::Attacks<Rooks>::MaskTable();
    static constexpr auto MaskBitCount = Generator::Attacks<Rooks>::MaskTableBitCount();

     GetAttack() = delete;
    ~GetAttack() = delete;
};

////////////////////////////////////////// QUEENS /////////////////////////////////////////////

template <>
struct GetAttack<Queens> final {
public:
    [[nodiscard]] static constexpr auto On(EnumSquare square, Bitboard occupancy) noexcept {
        return GetAttack<Bishops>::On(square, occupancy)
             | GetAttack<Rooks  >::On(square, occupancy);
    }
private:

     GetAttack() = delete;
    ~GetAttack() = delete;
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////






















//////////////////////////////////////// FOR TESTS //////////////////////////////////////////

template <EnumPiece Piece>
struct SliderAttacks final { };

template <>
struct SliderAttacks<Bishops> final {
public:
    [[nodiscard]] static constexpr auto On(EnumSquare square, Bitboard blockers) noexcept {
        Bitboard masks = 0ULL, mask = 0ULL;
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
    [[nodiscard]] static constexpr auto On(EnumSquare square, Bitboard blockers) noexcept {
        Bitboard masks = 0ULL, mask = 0ULL;
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
