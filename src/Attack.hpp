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
    [[nodiscard]] static _constexpr auto On(EnumSquare square, Bitboard occupancy) noexcept {
        constexpr auto FixedShift = (64-9);
        occupancy &= MaskTable[square];
        occupancy *= Magics<Bishops>[square];
        occupancy >>= FixedShift;
        return AttackTable[square][occupancy];
    }

private:
    static _constexpr auto AttackTable  = Generator::Attacks<Bishops>::AttackTable();
    static _constexpr auto MaskTable    = Generator::Attacks<Bishops>::MaskTable();
    // static constexpr auto MaskBitCount = Generator::Attacks<Bishops>::MaskTableBitCount();

     GetAttack() = delete;
    ~GetAttack() = delete;
};

////////////////////////////////////////// ROOKS //////////////////////////////////////////////

template <>
struct GetAttack<Rooks> final {
public:
    [[nodiscard]] static _constexpr auto On(EnumSquare square, Bitboard occupancy) noexcept {
        constexpr auto FixedShift = 64-12;
        occupancy &= MaskTable[square];
        occupancy *= Magics<Rooks>[square];
        occupancy >>= FixedShift;
        return AttackTable[square][occupancy];
    }

private:
    static _constexpr auto AttackTable  = Generator::Attacks<Rooks>::AttackTable();
    static _constexpr auto MaskTable    = Generator::Attacks<Rooks>::MaskTable();
    // static constexpr auto MaskBitCount = Generator::Attacks<Rooks>::MaskTableBitCount();

     GetAttack() = delete;
    ~GetAttack() = delete;
};

////////////////////////////////////////// QUEENS /////////////////////////////////////////////

template <>
struct GetAttack<Queens> final {
public:
    [[nodiscard]] static _constexpr auto On(EnumSquare square, Bitboard occupancy) noexcept {
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
