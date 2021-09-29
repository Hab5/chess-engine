#pragma once

#include "ZobristHashing.hpp"
#include "ChessEngine.hpp"
#include "GameState.hpp"
#include "GetAttack.hpp"
#include "Utils.hpp"

#include <cstdio>
#include <iomanip>

#define USE_HASH_TABLE

#if defined(USE_HASH_TABLE)

#define HASH_UPDATE_MOVE                                                          \
    Board.hash ^= ZobristHashing::Keys.Piece[piece+(Allies*6)-2][origin];         \
    Board.hash ^= ZobristHashing::Keys.Piece[piece+(Allies*6)-2][target];

#define HASH_UPDATE_CAPTURE                                                       \
    Board.hash ^= ZobristHashing::Keys.Piece[piece+(Enemies*6)-2][target];

#define HASH_UPDATE_CASTLING_RIGHTS                                               \
    Board.hash ^= ZobristHashing::Keys.Castle[Board.castling_rights.to_ulong()];

#define HASH_UPDATE_CASTLING_KING_ROOK                                            \
    constexpr auto KingRookTarget = Color == White ? f1 : f8;                     \
    Board.hash ^= ZobristHashing::Keys.Piece[Rooks+(Allies*6)-2][KingRook];       \
    Board.hash ^= ZobristHashing::Keys.Piece[Rooks+(Allies*6)-2][KingRookTarget];

#define HASH_UPDATE_CASTLING_QUEEN_ROOK                                           \
    constexpr auto QueenRookTarget = Color == White ? d1 : d8;                    \
    Board.hash ^= ZobristHashing::Keys.Piece[Rooks+(Allies*6)-2][QueenRook];      \
    Board.hash ^= ZobristHashing::Keys.Piece[Rooks+(Allies*6)-2][QueenRookTarget];

#define HASH_UPDATE_EN_PASSANT                                                    \
    Board.hash ^= ZobristHashing::Keys.EnPassant[Board.en_passant];

#define HASH_UPDATE_CAPTURE_EN_PASSANT                                            \
    Board.hash ^= ZobristHashing::Keys.Piece[Pawns+(Enemies*6)-2][target+Down];

#define HASH_UPDATE_PROMOTION                                                     \
    Board.hash ^= ZobristHashing::Keys.Piece[Pawns+(Allies*6)-2][origin];         \
    Board.hash ^= ZobristHashing::Keys.Piece[promotion+(Allies*6)-2][target];

#define HASH_UPDATE_SIDE                                                         \
    Board.hash ^= ZobristHashing::Keys.Side;

#else

#define HASH_UPDATE_MOVE
#define HASH_UPDATE_CAPTURE
#define HASH_UPDATE_CASTLING_RIGHTS
#define HASH_UPDATE_CASTLING_KING_ROOK
#define HASH_UPDATE_CASTLING_QUEEN_ROOK
#define HASH_UPDATE_CAPTURE_EN_PASSANT
#define HASH_UPDATE_EN_PASSANT
#define HASH_UPDATE_PROMOTION
#define HASH_UPDATE_SIDE

#endif

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

    template<EnumColor Color> [[nodiscard]]
    static inline auto Make(GameState& Board, const Move& move) noexcept {
        constexpr auto Allies    = Color, Enemies = ~Color;
        constexpr auto Down      = Allies == White ? South : North;
        constexpr auto KingRook  = Allies == White ? h1 : h8;
        constexpr auto QueenRook = Allies == White ? a1 : a8;
        constexpr auto Kk        = Allies == White ? 0 : 2;
        constexpr auto Qq        = Allies == White ? 1 : 3;

        const auto [piece, origin, target, flags] = move;

        if (Board.en_passant) HASH_UPDATE_EN_PASSANT;

        //////////////////////////////////////// QUIET ///////////////////////////////////////

        if (flags == Quiet) { HASH_UPDATE_SIDE; HASH_UPDATE_MOVE;

            Board.to_play = Enemies;
            Board.en_passant = EnumSquare(0);
            Board[Allies] ^= (Board[piece] ^= (origin|target), (origin|target));

            if (piece == Rooks) {
                HASH_UPDATE_CASTLING_RIGHTS;
                if (origin == KingRook ) Board.castling_rights[Kk] = 0;
                if (origin == QueenRook) Board.castling_rights[Qq] = 0;
                HASH_UPDATE_CASTLING_RIGHTS;
            } else if (piece == King) {
                HASH_UPDATE_CASTLING_RIGHTS;
                Board.castling_rights[Kk] = 0;
                Board.castling_rights[Qq] = 0;
                HASH_UPDATE_CASTLING_RIGHTS;
            }

            return not GameState::InCheck<Allies>(Board, Utils::IndexLS1B(
                Board[King] & Board[Allies])
            );


        } else { Board.en_passant = EnumSquare(0);

         ////////////////////////////////////// CAPTURE //////////////////////////////////////

            if (flags & Capture) {
                for (int piece = Pawns; piece <= King; ++piece) {

                    if (Board[piece] & target) {
                        Board[Enemies] ^= (Board[piece] ^= target, target);
                        HASH_UPDATE_CAPTURE;
                        if (piece == Rooks) {
                            constexpr auto EnemyKingRook  = Allies == White ? h8 : h1;
                            constexpr auto EnemyQueenRook = Allies == White ? a8 : a1;
                            constexpr auto EnemyKk        = Allies == White ? 2  : 0 ;
                            constexpr auto EnemyQq        = Allies == White ? 3  : 1 ;

                            if (target == EnemyKingRook) {
                                HASH_UPDATE_CASTLING_RIGHTS;
                                Board.castling_rights[EnemyKk] = 0;
                                HASH_UPDATE_CASTLING_RIGHTS;
                            }

                            else if (target == EnemyQueenRook) {
                                HASH_UPDATE_CASTLING_RIGHTS;
                                Board.castling_rights[EnemyQq] = 0;
                                HASH_UPDATE_CASTLING_RIGHTS;
                            }
                        }
                    }
                }
            }

        ////////////////////////////////////// SPECIAL ///////////////////////////////////////

            else if (flags == DoublePush) {
                Board.en_passant = target + Down;
                HASH_UPDATE_EN_PASSANT;
            }

            else if (flags == CastleKing)  { HASH_UPDATE_CASTLING_KING_ROOK;
                constexpr auto CastleK = Allies == White ? (h1|f1) : (h8|f8);
                Board[Allies] ^= (Board[Rooks] ^= CastleK, CastleK);
            }

            else if (flags == CastleQueen) { HASH_UPDATE_CASTLING_QUEEN_ROOK;
                constexpr auto CastleQ = Allies == White ? (a1|d1) : (a8|d8);
                Board[Allies] ^= (Board[Rooks] ^= CastleQ, CastleQ);
            }

            if (flags == EnPassant)        { HASH_UPDATE_CAPTURE_EN_PASSANT;
                Board[Enemies] ^= (Board[Pawns] ^= target+Down, target+Down);
            }

            else if (flags & PromotionKnight) {
                const auto promotion =
                flags == PromotionBishop || flags == XPromotionBishop ? Bishops :
                flags == PromotionRook   || flags == XPromotionRook   ? Rooks   :
                flags == PromotionQueen  || flags == XPromotionQueen  ? Queens  : Knights;

                HASH_UPDATE_SIDE; HASH_UPDATE_PROMOTION;

                Board[Allies] ^= (Board[Pawns] ^= origin, (origin|target));
                Board[promotion] |= target; Board.to_play = Enemies;

                return not GameState::InCheck<Allies>(Board, Utils::IndexLS1B(
                    Board[King] & Board[Allies])
                );
            }

            if (piece == Rooks) {
                HASH_UPDATE_CASTLING_RIGHTS;
                if (origin == KingRook ) Board.castling_rights[Kk] = 0;
                if (origin == QueenRook) Board.castling_rights[Qq] = 0;
                HASH_UPDATE_CASTLING_RIGHTS;
            } else if (piece == King) {
                HASH_UPDATE_CASTLING_RIGHTS;
                Board.castling_rights[Kk] = 0;
                Board.castling_rights[Qq] = 0;
                HASH_UPDATE_CASTLING_RIGHTS;
            }

        //////////////////////////////////////////////////////////////////////////////////////

            HASH_UPDATE_SIDE; HASH_UPDATE_MOVE;

            Board.to_play  = Enemies;
            Board[Allies] ^= (Board[piece] ^= (origin|target), (origin|target));

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
