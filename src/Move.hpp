#pragma once

#include "ChessEngine.hpp"
#include "GameState.hpp"
#include "GetAttack.hpp"
#include "Utils.hpp"

#include <iomanip>

enum EnumMoveFlags: std::uint8_t {
    Quiet            = 0b0000,
    DoublePush       = 0b0001,
    CastleKing       = 0b0010,
    CastleQueen      = 0b0011,
    Capture          = 0b0100,
    EnPassant        = 0b0101,
    PromotionKnight  = 0b1000,
    PromotionBishop  = 0b1001,
    PromotionRook    = 0b1010,
    PromotionQueen   = 0b1011,
    XPromotionKnight = 0b1100,
    XPromotionBishop = 0b1101,
    XPromotionRook   = 0b1110,
    XPromotionQueen  = 0b1111,

};

inline std::ostream& operator<<(std::ostream& os, const EnumMoveFlags& flags) {
    return (os << std::string(
    flags == Quiet            ? "Quiet"                     :
    flags == DoublePush       ? "DoublePush"                :
    flags == CastleKing       ? "CastleKing"                :
    flags == CastleQueen      ? "CastleQueen"               :
    flags == Capture          ? "Capture"                   :
    flags == EnPassant        ? "EnPassant"                 :
    flags == PromotionKnight  ? "PromotionKnight"           :
    flags == PromotionBishop  ? "PromotionBishops"          :
    flags == PromotionRook    ? "PromotionRook"             :
    flags == PromotionQueen   ? "PromotionQueen"            :
    flags == XPromotionKnight ? "Capture & PromotionKnight" :
    flags == XPromotionBishop ? "Capture & PromotionBishop" :
    flags == XPromotionRook   ? "Capture & PromotionRook"   :
    flags == XPromotionQueen  ? "Capture & PromotionQueen"  : "Empty"));
}

constexpr inline EnumMoveFlags operator+(EnumMoveFlags lhs, EnumMoveFlags rhs) noexcept {
    return static_cast<EnumMoveFlags>(static_cast<int>(lhs)+static_cast<int>(rhs));
}

constexpr inline EnumMoveFlags operator|(EnumMoveFlags lhs, EnumMoveFlags rhs) noexcept {
    return lhs+rhs;
}

struct Move final {
    EnumPiece     piece;
    EnumSquare    origin;
    EnumSquare    target;
    EnumMoveFlags flags;

    template <EnumPiece Piece>
    [[nodiscard]] static inline constexpr auto Encode
    (EnumSquare origin, EnumSquare target, EnumMoveFlags flags) noexcept {
        return Move {
            .piece  = Piece,
            .origin = origin,
            .target = target,
            .flags  = flags
        };
    }

    template<EnumColor Color> [[nodiscard]] //__attribute__((always_inline))
    static inline auto Make(GameState& Board, const Move& move) noexcept {
        constexpr auto Allies    = Color, Enemies = ~Color;
        constexpr auto Down      = Allies == White ? South : North;
        constexpr auto KingRook  = Allies == White ? h1 : h8;
        constexpr auto QueenRook = Allies == White ? a1 : a8;
        constexpr auto CastleK   = Allies == White ? (h1|f1) : (h8|f8);
        constexpr auto CastleQ   = Allies == White ? (a1|d1) : (a8|d8);
        constexpr auto Kk        = Allies == White ? 0 : 2;
        constexpr auto Qq        = Allies == White ? 1 : 3;

        const auto [piece, origin, target, flags] = move;

        //////////////////////////////////////// QUIET ///////////////////////////////////////

        if (flags == Quiet) {
            Board.to_play    = Enemies;
            Board.en_passant = EnumSquare(0);
            Board[Allies] ^= (Board[piece] ^= (origin|target), (origin|target));
            if (piece == Rooks) {
                if (origin == KingRook ) Board.castling_rights[Kk] = 0;
                if (origin == QueenRook) Board.castling_rights[Qq] = 0;
            } else if (piece == King) {
                Board.castling_rights[Kk] = 0;
                Board.castling_rights[Qq] = 0;
            } return not GameState::InCheck<Allies>(
                   Board, Utils::IndexLS1B(Board[King] & Board[Allies]));
        } else { Board.en_passant = EnumSquare(0);

         ////////////////////////////////////// CAPTURE //////////////////////////////////////

            if (flags & Capture) {
                for (auto set = &Board[Pawns]; set != &Board[King]; ++set)
                    if (*set & target) Board[Enemies] ^= (*set ^= target, target);
            }

        ////////////////////////////////////// SPECIAL ///////////////////////////////////////

            else if (flags == DoublePush)
                Board.en_passant = target + Down;

            else if (flags == CastleKing)
                Board[Allies] ^= (Board[Rooks] ^= CastleK, CastleK);

            else if (flags == CastleQueen)
                Board[Allies] ^= (Board[Rooks] ^= CastleQ, CastleQ);

            if (flags == EnPassant)
                Board[Enemies] ^= (Board[Pawns] ^= target+Down, target+Down);

            else if (flags & PromotionKnight) {
                const auto promotion =
                flags == PromotionBishop || flags == XPromotionBishop ? Bishops :
                flags == PromotionRook   || flags == XPromotionRook   ? Rooks   :
                flags == PromotionQueen  || flags == XPromotionQueen  ? Queens  : Knights;
                Board[Allies] ^= (Board[Pawns] ^= origin, (origin|target));
                Board[promotion] |= target; Board.to_play = Enemies;
                return not GameState::InCheck<Allies>(Board, Utils::IndexLS1B(
                    Board[King] & Board[Allies])
                );
            }

            if (piece == Rooks) {
                if (origin == KingRook ) Board.castling_rights[Kk] = 0;
                if (origin == QueenRook) Board.castling_rights[Qq] = 0;
            } else if (piece == King) {
                Board.castling_rights[Kk] = 0;
                Board.castling_rights[Qq] = 0;
            }

        //////////////////////////////////////////////////////////////////////////////////////

            Board.to_play  = Enemies;
            Board[Allies] ^= (Board[piece] ^= (origin|target), (origin|target));
            return not GameState::InCheck<Allies>(Board, Utils::IndexLS1B(
                Board[King] & Board[Allies])
            );
        }
    }

    friend inline std::ostream& operator<<(std::ostream& os, const Move& move) {
        return os << std::left << std::setw(8) << move.piece << std::setw(0) << "| "
                  << move.origin << '-' << move.target << " | " << move.flags;
    }
};
