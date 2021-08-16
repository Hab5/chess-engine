#pragma once

#include "AttackGeneration.hpp"
#include "ChessEngine.hpp"
#include "Utils.hpp"

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
        // STRUCT OF ARRAY
        occupancy &= Magics.Masks[square];
        occupancy *= Magics.Numbers[square];
        occupancy >>= (64-9);
        return Magics.Attacks[square][occupancy];

        // ARRAY OF STRUCT
        // occupancy &= Magics[square].Mask;
        // occupancy *= Magics[square].Number;
        // occupancy >>= (64-9);
        // return Magics[square].Attack[occupancy];
    }

private:
    const static _constexpr auto Magics = Generator::Attacks<Bishops>::Magics_SOA();
    // const static _constexpr auto Magics = Generator::Attacks<Bishops>::Magics_AOS();

     GetAttack() = delete;
    ~GetAttack() = delete;
};

////////////////////////////////////////// ROOKS //////////////////////////////////////////////

template <>
struct GetAttack<Rooks> final {
public:
    [[nodiscard]] static _constexpr auto On(EnumSquare square, Bitboard occupancy) noexcept {
        // STRUCT OF ARRAY
        occupancy &= Magics.Masks[square];
        occupancy *= Magics.Numbers[square];
        occupancy >>= (64-12);
        return Magics.Attacks[square][occupancy];

        // ARRAY OF STRUCT
        // occupancy &= Magics[square].Mask;
        // occupancy *= Magics[square].Number;
        // occupancy >>= (64-12);
        // return Magics[square].Attack[occupancy];
    }

private:
    const static _constexpr auto Magics = Generator::Attacks<Rooks>::Magics_SOA();
    // const static _constexpr auto Magics = Generator::Attacks<Rooks>::Magics_AOS();

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
