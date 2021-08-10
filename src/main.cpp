#include "ChessEngine.hpp"
#include "ChessBoard.hpp"
#include "Attack.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <ostream>
#include <type_traits>
#include <vector>
#include <array>
#include <iomanip>

enum EnumMoveFlags {
    Quiet                  = 0b0000,
    DoublePawnPush         = 0b0001,
    CastleKing             = 0b0010,
    CastleQueen            = 0b0011,
    Capture                = 0b0100,
    EnPassant              = 0b0101,
    KnightPromotion        = 0b1000,
    BishopPromotion        = 0b1001,
    RookPromotion          = 0b1010,
    QueenPromotion         = 0b1011,
    KnightPromotionCapture = 0b1100,
    BishopPromotionCapture = 0b1101,
    RookPromotionCapture   = 0b1110,
    QueenPromotionCapture  = 0b1111,

};

constexpr inline EnumMoveFlags operator+(EnumMoveFlags lhs, EnumMoveFlags rhs) noexcept {
    return static_cast<EnumMoveFlags>(static_cast<int>(lhs)+static_cast<int>(rhs));
}

constexpr inline EnumMoveFlags operator|(EnumMoveFlags lhs, EnumMoveFlags rhs) noexcept {
    return lhs+rhs;
}

std::ostream& operator<<(std::ostream& os, const EnumMoveFlags& flag) {
    return os << std::string((
        flag == Quiet                  ? "Quiet"                   :
        flag == DoublePawnPush         ? "DoublePawnPush"          :
        flag == CastleKing             ? "CastleKing"              :
        flag == CastleQueen            ? "CastleQueen"             :
        flag == Capture                ? "Capture"                 :
        flag == EnPassant              ? "EnPassant"               :
        flag == KnightPromotion        ? "KnightPromotion"         :
        flag == BishopPromotion        ? "BishopPromotion"         :
        flag == RookPromotion          ? "RookPromotion"           :
        flag == QueenPromotion         ? "QueenPromotion"          :
        flag == KnightPromotionCapture ? "Capture|KnightPromotion" :
        flag == BishopPromotionCapture ? "Capture|BishopPromotion" :
        flag == RookPromotionCapture   ? "Capture|RookPromotion"   :
        flag == QueenPromotionCapture  ? "Capture|QueenPromotion"  : "None"
    ));
}

#define POSITION "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
ChessBoard Board(STARTING_POSITION);

template <EnumColor Color>
inline bool IsSquareAttackedBy(EnumSquare square) noexcept {
    constexpr auto Other = ~Color;
    const auto occ = Board[White] | Board[Black];
    return (
        GetAttack<Other, Pawns>::On(square     ) & (Board[Pawns  ] & Board[Color]) ? true :
        GetAttack<Knights     >::On(square     ) & (Board[Knights] & Board[Color]) ? true :
        GetAttack<Bishops     >::On(square, occ) & (Board[Bishops] & Board[Color]) ? true :
        GetAttack<Rooks       >::On(square, occ) & (Board[Rooks  ] & Board[Color]) ? true :
        GetAttack<Queens      >::On(square, occ) & (Board[Queens ] & Board[Color]) ? true :
        GetAttack<King        >::On(square     ) & (Board[King   ] & Board[Color]) ? true :
        false
    );
}

template <EnumColor Color>
inline std::uint64_t GetAttackedSquares() noexcept {
    std::uint64_t attacked = 0ULL;
    for (EnumSquare square = EnumSquare::a1; square <= EnumSquare::h8; ++square)
        if (IsSquareAttackedBy<Color>(square))
            attacked |= square;
    return attacked;
}

struct Move final {
    std::uint16_t encoded;
    EnumPiece     piece;
public:

    template <EnumPiece Piece>
    [[nodiscard]] static constexpr auto Encode
    (EnumSquare origin, EnumSquare target, EnumMoveFlags flags) noexcept {
        return Move {
        .encoded = static_cast<std::uint16_t>((
                  (( flags  & 0xf ) << 12)
                | ((+origin & 0x3f) << 6 )
                | ((+target & 0x3f) << 0 ))),
        .piece   = Piece
        };

    }

    [[nodiscard]] static constexpr auto Decode(const Move& move) noexcept
    -> std::tuple<EnumPiece, EnumSquare, EnumSquare, EnumMoveFlags> {
        return { move.piece,
            static_cast<EnumSquare   >((move.encoded >> 6 ) & 0x3f), // origin
            static_cast<EnumSquare   >((move.encoded >> 0 ) & 0x3f), // target
            static_cast<EnumMoveFlags>((move.encoded >> 12) & 0xf)   // flags
        };
    }

    template<EnumColor Color> __attribute__((always_inline))
    static inline auto Make(Move& move) noexcept {
        const auto [piece, origin, target, flags] = Move::Decode(move);
        constexpr auto OtherColor        = ~Color;

        Board.en_passant = a1;
        if (flags & Capture) {
            std::for_each(Board.pieces.begin()+2, Board.pieces.end(),
            [&, t=target](auto& set) { if (set & t) Board[OtherColor] ^= (set ^= t, t); });
        } else if (flags == DoublePawnPush) {
            constexpr auto EnPassantMask = Color == White ? South : North;
            Board.en_passant = target + EnPassantMask;
        } else if (flags == CastleKing) {
            constexpr auto CastleKingMask =  Color == White ? (h1|f1) : (h8|f8);
            Board[Color] ^= (Board[Rooks] ^= CastleKingMask, CastleKingMask);
        } else if (flags == CastleQueen) {
            constexpr auto CastleQueenMask =  Color == White ? (a1|d1) : (a8|d8);
            Board[Color] ^= (Board[Rooks] ^= CastleQueenMask, CastleQueenMask);
        }

        if (flags == EnPassant) {
            constexpr auto EnPassantMask = Color == White ? South : North;
            const auto to_erase = target + EnPassantMask;
            Board[OtherColor] ^= (Board[Pawns] ^= to_erase, to_erase);
        }


        if (flags & KnightPromotion) {
            const auto promoted_to =
              flags == BishopPromotion || flags == BishopPromotionCapture ? Bishops :
              flags == RookPromotion   || flags == RookPromotionCapture   ? Rooks   :
              flags == QueenPromotion  || flags == QueenPromotionCapture  ? Queens  : Knights;
            Board[promoted_to] |= target;
            Board[Pawns] ^= target;

        }

        auto mask = (origin|target);
        Board[Color] ^= (Board[piece] ^= mask, mask);

        if (piece == King) {
            constexpr auto CastlingRightsIdx = Color == White ? 0 : 2;
            Board.castling_rights[CastlingRightsIdx]   = 0;
            Board.castling_rights[CastlingRightsIdx+1] = 0;
        }
        if (piece == Rooks) {
            if constexpr (Color == White) {
                if (origin == h1) Board.castling_rights[0] = 0; // K
                if (origin == a1) Board.castling_rights[1] = 0; // Q
            } else {
                if (origin == h8) Board.castling_rights[2] = 0; // k
                if (origin == a8) Board.castling_rights[3] = 0; // q
            }
        }

        Board.to_play = static_cast<EnumColor>(!Board.to_play);

        auto king_square = Utils::IndexLS1B(Board[King] & Board[Color]);
        return !IsSquareAttackedBy<OtherColor>(king_square);
        // return true;
    }

    friend inline std::ostream& operator<<(std::ostream& os, const Move& move) {
        auto [piece, origin, target, flags] = Move::Decode(move);

        return os << std::left << std::setw(8) << piece << std::setw(0) << "| "
                  << origin << '-' << target << " | " << flags;
    }

    friend inline bool operator>(const Move& move, const Move& other) {
        return move.encoded > other.encoded;
    }
};


class MoveGen final {
public:
    template <EnumColor Color> __attribute__((always_inline))
    static inline auto All() noexcept -> std::tuple<std::array<Move, 218>, std::uint8_t> {
        std::array<Move, 218> moves { };

        auto iterator = moves.begin();
        GenerateMoves<Color, Pawns   >(iterator);
        GenerateMoves<Color, Knights >(iterator);
        GenerateMoves<Color, Bishops >(iterator);
        GenerateMoves<Color, Rooks   >(iterator);
        GenerateMoves<Color, Queens  >(iterator);
        GenerateMoves<Color, King    >(iterator);

        auto nmoves = std::distance(moves.begin(), iterator);

        return { moves, nmoves };
    }

    template <EnumColor Color>
    static inline std::uint64_t Perft(int depth) noexcept {
        std::uint64_t nodes = 0;
        if (depth == 0) return 1ULL;

        ChessBoard Old = Board;
        constexpr auto Other = ~Color;
        auto [MoveList, nmoves] = MoveGen::All<Color>();
        for (auto move = 0; move < nmoves; move++) {
            if (Move::Make<Color>(MoveList[move]))
                nodes += Perft<Other>(depth-1);
            Board = Old;
        }

        return nodes;
    }

private:
    template <EnumColor Color, EnumPiece Piece> __attribute__((always_inline))
    static inline auto GenerateMoves(std::array<Move, 218>::iterator& Moves) noexcept {
        auto set              = (Board[Color] & Board[ Piece]);
        auto occupancy        = (Board[Color] | Board[!Color]);

        while (set) {
        EnumSquare origin = Utils::PopLS1B(set);

        if constexpr(Piece == Pawns) {
            constexpr auto promotion_rank = (Color == White ? Rank_8 : Rank_1);
            constexpr auto starting_rank  = (Color == White ? Rank_2 : Rank_7);
            constexpr auto up             = (Color == White ? North  : South );

            EnumSquare target = origin + up;
            if ((target & promotion_rank) & ~occupancy) {
                *Moves++ = Move::Encode<Piece>(origin, target, KnightPromotion);
                *Moves++ = Move::Encode<Piece>(origin, target, BishopPromotion);
                *Moves++ = Move::Encode<Piece>(origin, target, RookPromotion  );
                *Moves++ = Move::Encode<Piece>(origin, target, QueenPromotion );
            } else if (target & ~occupancy) {
                *Moves++ = Move::Encode<Piece>(origin, target, Quiet);
                if ((origin & starting_rank) && ((target+up) & ~occupancy))
                    *Moves++ = Move::Encode<Piece>(origin, (target+up), DoublePawnPush);
            }

            auto attacks = GetAttack<Color, Piece>::On(origin) & Board[!Color];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
                if (attack & promotion_rank) { --Moves;
                    *Moves++ = Move::Encode<Piece>(origin, attack, KnightPromotion | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, BishopPromotion | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, RookPromotion   | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, QueenPromotion  | Capture);
                }
            }

            if (Board.GetEnPassant()) {
                if (GetAttack<Color, Piece>::On(origin) & Board.GetEnPassant())
                    *Moves++ = Move::Encode<Piece>(origin, Board.GetEnPassant(), EnPassant);
            }
        }

        if constexpr(Piece == King || Piece == Knights) {
            auto attacks = GetAttack<Piece>::On(origin) & ~Board[Color];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                if (attack & ~Board[!Color])
                     *Moves++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
            }

            if constexpr(Piece == King) {
                constexpr auto king = (Color == White ? e1:e8);

                constexpr auto Kk = (Color == White ? 0 : 2);
                if (Board.GetCastlingRights()[Kk]) {
                    if (!(((king+1) | (king+2)) & occupancy))
                        if (!IsSquareAttackedBy<(~Color)>(king)
                        &&  !IsSquareAttackedBy<(~Color)>(king+1)
                        &&  !IsSquareAttackedBy<(~Color)>(king+2))
                            *Moves++ = Move::Encode<Piece>(origin, king+2, CastleKing);
                }

                constexpr auto Qq = (Color == White ? 1 : 3);
                if (Board.GetCastlingRights()[Qq]) {
                    if (!(((king-1) | (king-2) | (king-3)) & occupancy))
                        if (!IsSquareAttackedBy<(~Color)>(king)
                        &&  !IsSquareAttackedBy<(~Color)>(king-1)
                        &&  !IsSquareAttackedBy<(~Color)>(king-2))
                            *Moves++ = Move::Encode<Piece>(origin, king-2, CastleQueen);
                }
            }
        }

        if constexpr(Piece == Bishops || Piece == Rooks || Piece == Queens) {
            auto attacks = Bitboard(0);
            if constexpr(Piece == Queens)
                attacks  = (GetAttack<Bishops>::On(origin, occupancy)
                         |  GetAttack<Rooks  >::On(origin, occupancy)) & ~Board[Color];
            else attacks =  GetAttack<Piece  >::On(origin, occupancy)  & ~Board[Color];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                if (attack & ~Board[!Color])
                     *Moves++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
            }
        }

        }
    }

     MoveGen()=delete;
    ~MoveGen()=delete;
};

#include <chrono>

int main(int argc, char* argv[]) { (void)argc;
    std::cout << Board << std::endl;

    auto started  = std::chrono::steady_clock::now();
    auto nodes    = MoveGen::Perft<White>(std::atoi(argv[1]));
    auto finished = std::chrono::steady_clock::now();

    auto ms       = std::chrono::duration_cast
                   <std::chrono::milliseconds>
                   (finished-started).count();

    long nps      = nodes / (ms / 1000.00f);

    std::cout.imbue(std::locale(""));
    std::cout << "nodes: " << nodes << std::endl
              << "nps  : " << nps   << std::endl
              << "ms   : " << ms    << std::endl;
    return 0;
}
