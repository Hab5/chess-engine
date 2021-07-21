#pragma once

#include "ChessEngine.hpp"

#include <array>

namespace Generator {

using AttackTable = std::array<std::uint64_t, 64>;

template <EnumColor color, EnumPiece type>
class Attack final {
public:
    static constexpr auto Get() noexcept {
        switch(type) {
        case EnumPiece::Pawns:   return Attack::Pawn();
        case EnumPiece::Knights: return Attack::Knight();
        case EnumPiece::Bishops: return Attack::Bishop();
        case EnumPiece::Rooks:   return Attack::Rook();
        case EnumPiece::Queens:  return Attack::Queen();
        case EnumPiece::King:    return Attack::King();
        }
    }

private:

    [[nodiscard]] static constexpr std::array<std::uint64_t, 64> Pawn() noexcept {
        std::array<std::uint64_t, 64> masks { };
        for (int square = 0; square < 64; square++) {
            std::uint64_t pawn = 0ULL | (1ULL << square);
            switch (color) {
            case White:
                if ((pawn << 9) & ~File_A) masks[square] |= (pawn << 9);
                if ((pawn << 7) & ~File_H) masks[square] |= (pawn << 7);
                break;
            case Black:
                if ((pawn >> 7) & ~File_A) masks[square] |= (pawn >> 7);
                if ((pawn >> 9) & ~File_H) masks[square] |= (pawn >> 9);
                break;
            }
        } return masks;
    }

    [[nodiscard]] static constexpr AttackTable Knight() noexcept { return {0ULL}; }
    [[nodiscard]] static constexpr AttackTable Bishop() noexcept { return {0ULL}; }
    [[nodiscard]] static constexpr AttackTable Rook() noexcept { return {0ULL}; }
    [[nodiscard]] static constexpr AttackTable Queen() noexcept { return {0ULL}; }
    [[nodiscard]] static constexpr AttackTable King() noexcept { return {0ULL}; }

     Attack() = delete;
    ~Attack() = delete;
};

}

template <EnumColor Color, EnumPiece Piece>
struct Attack final {
public:
    [[nodiscard]] static constexpr auto At(std::size_t square) noexcept {
        return MaskTable[square];
    };

private:
    static constexpr auto MaskTable = Generator::Attack<Color, Piece>::Get();

     Attack() = delete;
    ~Attack() = delete;
};
