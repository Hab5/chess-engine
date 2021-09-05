#pragma once

#include "TranspositionTable.hpp"
#include "ChessEngine.hpp"
#include "GameState.hpp"
#include "GetAttack.hpp"
#include "Utils.hpp"

#include <cstdio>
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

        if (Board.en_passant)
            Board.hash ^= Zobrist::Keys.EnPassant[Board.en_passant];

        //////////////////////////////////////// QUIET ///////////////////////////////////////

        if (flags == Quiet) {
            Board.hash ^= Zobrist::Keys.Side;
            Board.hash ^= Zobrist::Keys.Piece[piece+(Allies*6)-2][origin];
            Board.hash ^= Zobrist::Keys.Piece[piece+(Allies*6)-2][target];

            Board.to_play    = Enemies;
            Board.en_passant = EnumSquare(0);
            Board[Allies] ^= (Board[piece] ^= (origin|target), (origin|target));

            if (piece == Rooks) { // FIX CASTLING THINGY
                Board.hash ^= Zobrist::Keys.Castle[Board.castling_rights.to_ullong()]; 
                if (origin == KingRook ) Board.castling_rights[Kk] = 0;
                if (origin == QueenRook) Board.castling_rights[Qq] = 0;
                Board.hash ^= Zobrist::Keys.Castle[Board.castling_rights.to_ullong()];

            } else if (piece == King) {
                Board.hash ^= Zobrist::Keys.Castle[Board.castling_rights.to_ullong()];
                Board.castling_rights[Kk] = 0, Board.castling_rights[Qq] = 0;
                Board.hash ^= Zobrist::Keys.Castle[Board.castling_rights.to_ullong()];
            }

            return not GameState::InCheck<Allies>(Board, Utils::IndexLS1B(
                Board[King] & Board[Allies])
            );

            if (Board.hash != Zobrist::Hash(Board)) std::cout << "FAILED QUIET HASH\n";

        } else { Board.en_passant = EnumSquare(0);

         ////////////////////////////////////// CAPTURE //////////////////////////////////////

            if (flags & Capture) {
                for (int piece = Pawns; piece <= King; ++piece) {
                    if (Board[piece] & target) {
                        Board[Enemies] ^= (Board[piece] ^= target, target);
                        Board.hash ^= Zobrist::Keys.Piece[piece+(Enemies*6)-2][target];
                    }
                }
            }

        ////////////////////////////////////// SPECIAL ///////////////////////////////////////

            else if (flags == DoublePush) {
                Board.en_passant = target + Down;
                Board.hash ^= Zobrist::Keys.EnPassant[Board.en_passant];
            }

            else if (flags == CastleKing) {
                constexpr auto KingRookDest = Color == White ? h1 : h8;
                Board.hash ^= Zobrist::Keys.Piece[Rooks+(Allies*6)-2][KingRook];
                Board.hash ^= Zobrist::Keys.Piece[Rooks+(Allies*6)-2][KingRookDest];
                Board[Allies] ^= (Board[Rooks] ^= CastleK, CastleK);
            }

            else if (flags == CastleQueen) {
                constexpr auto QueenRookDest = Color == White ? a1 : a8;
                Board.hash ^= Zobrist::Keys.Piece[Rooks+(Allies*6)-2][QueenRook];
                Board.hash ^= Zobrist::Keys.Piece[Rooks+(Allies*6)-2][QueenRookDest];
                Board[Allies] ^= (Board[Rooks] ^= CastleQ, CastleQ);
            }

            if (flags == EnPassant) {
                Board[Enemies] ^= (Board[Pawns] ^= target+Down, target+Down);
                Board.hash ^= Zobrist::Keys.Piece[Pawns+(Enemies*6)-2][target+Down];
            }

            else if (flags & PromotionKnight) {
                const auto promotion =
                flags == PromotionBishop || flags == XPromotionBishop ? Bishops :
                flags == PromotionRook   || flags == XPromotionRook   ? Rooks   :
                flags == PromotionQueen  || flags == XPromotionQueen  ? Queens  : Knights;


                Board.hash ^= Zobrist::Keys.Side;
                Board.hash ^= Zobrist::Keys.Piece[Pawns+(Allies*6)-2][origin];
                Board.hash ^= Zobrist::Keys.Piece[promotion+(Allies*6)-2][target];

                Board[Allies] ^= (Board[Pawns] ^= origin, (origin|target));
                Board[promotion] |= target; Board.to_play = Enemies;

                if (Board.hash != Zobrist::Hash(Board)) std::cout << "FAILED PROMOTION HASH\n";

                return not GameState::InCheck<Allies>(Board, Utils::IndexLS1B(
                    Board[King] & Board[Allies])
                );
            }

            if (piece == Rooks) {
                Board.hash ^= Zobrist::Keys.Castle[Board.castling_rights.to_ullong()];
                if (origin == KingRook ) Board.castling_rights[Kk] = 0;
                if (origin == QueenRook) Board.castling_rights[Qq] = 0;
                Board.hash ^= Zobrist::Keys.Castle[Board.castling_rights.to_ullong()];
            } else if (piece == King) {
                Board.hash ^= Zobrist::Keys.Castle[Board.castling_rights.to_ullong()];
                Board.castling_rights[Kk] = 0;
                Board.castling_rights[Qq] = 0;
                Board.hash ^= Zobrist::Keys.Castle[Board.castling_rights.to_ullong()];
            }

        //////////////////////////////////////////////////////////////////////////////////////

            Board.hash ^= Zobrist::Keys.Side;
            Board.hash ^= Zobrist::Keys.Piece[piece+(Allies*6)-2][origin];
            Board.hash ^= Zobrist::Keys.Piece[piece+(Allies*6)-2][target];

            Board.to_play  = Enemies;
            Board[Allies] ^= (Board[piece] ^= (origin|target), (origin|target));


            if (Board.hash != Zobrist::Hash(Board)) {
                std::cout << "FAILED: " << move.flags << '\n';
                std::cout << Board << '\n'; getchar();
            }
            Board.hash = Zobrist::Hash(Board); // TEMPORARY

            return not GameState::InCheck<Allies>(Board, Utils::IndexLS1B(
                Board[King] & Board[Allies])
            );
        }
    }

    friend inline std::ostream& operator<<(std::ostream& os, const Move& move) {
        return os << move.origin << move.target
                  << ((move.flags  &  PromotionKnight) ?
                     ((move.flags ==  PromotionKnight) ||
                      (move.flags == (PromotionKnight  | Capture))  ? "n" :
                     ((move.flags ==  PromotionBishop) ||
                      (move.flags == (PromotionBishop  | Capture))) ? "b" :
                     ((move.flags ==  PromotionRook)   ||
                      (move.flags == (PromotionRook    | Capture))) ? "r" :
                     ((move.flags ==  PromotionQueen)  ||
                      (move.flags == (PromotionQueen   | Capture))) ? "q" :
                      "") : "");
    }
};

using MoveList = std::array<Move, 218>;
