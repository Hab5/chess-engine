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

#define POSITION "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
ChessBoard Board(STARTING_POSITION);

template <EnumColor Color>
inline bool IsSquareAttacked(EnumSquare square) noexcept {
    constexpr auto Other = EnumColor(!Color);
    auto occ = Board[White] | Board[Black];
    return (
        GetAttack<Other, Pawns>::On(square)      & (Board[Pawns  ] & Board[Color]) ? true :
        GetAttack<Knights     >::On(square)      & (Board[Knights] & Board[Color]) ? true :
        GetAttack<Bishops     >::On(square, occ) & (Board[Bishops] & Board[Color]) ? true :
        GetAttack<Rooks       >::On(square, occ) & (Board[Rooks  ] & Board[Color]) ? true :
        GetAttack<Queens      >::On(square, occ) & (Board[Queens ] & Board[Color]) ? true :
        GetAttack<King        >::On(square)      & (Board[King   ] & Board[Color]) ? true :
        false
    );
}

template <EnumColor Color>
inline std::uint64_t GetAttackedSquares() noexcept {
    std::uint64_t attacked = 0ULL;
    for (EnumSquare square = EnumSquare::a1; square <= EnumSquare::h8; ++square)
        if (IsSquareAttacked<Color>(square))
            attacked |= square;
    return attacked;
}

struct Move final {
    EnumPiece     piece;
    std::uint16_t encoded;
public:

    template <EnumPiece Piece>
    [[nodiscard]] static constexpr auto Encode
    (EnumSquare origin, EnumSquare target, EnumMoveFlags flags) noexcept {
        return Move {
        .piece   = Piece,
        .encoded = static_cast<std::uint16_t>((
                  (( flags  & 0xf ) << 12)
                | ((+origin & 0x3f) << 6 )
                | ((+target & 0x3f) << 0 )))
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

    inline auto Make() noexcept {
        const auto [piece, origin, target, flags] = Move::Decode(*this);

        const auto color = Board.to_play;
        const auto other = !color;

        if (flags & Capture) {
            std::for_each(Board.pieces.begin()+2, Board.pieces.end(),
                [&, t=target](auto& set) {
                    if (set & t) Board[other] ^= (set ^= t, t);
                }
            );
        }

        else if (flags == CastleKing) {
            const auto mask = (color == White ? (h1|f1) : (h8|f8));
            Board[color] ^= (Board[Rooks] ^= mask, mask);
        }

        else if (flags == CastleQueen) {
            const auto mask = (color == White ? (a1|d1) : (a8|d8));
            Board[color] ^= (Board[Rooks] ^= mask, mask);
        }

        if (flags & KnightPromotion) {
            const auto promoted_to =
                (flags == BishopPromotion || flags == BishopPromotionCapture ? Bishops :
                 flags == RookPromotion   || flags == RookPromotionCapture   ? Rooks   :
                 flags == QueenPromotion  || flags == QueenPromotionCapture  ? Queens  : Knights);
            Board[promoted_to] |= target;
            Board[Pawns] ^= target;

        }

        if (flags == EnPassant) {
            const auto to_erase = target + (color == White ? South : North);
            Board[other] ^= (Board[Pawns] ^= to_erase, to_erase);
        }

        Board.en_passant = a1; // no en_passant

        if (flags == DoublePawnPush) {
            Board.en_passant = target + (color == White ? South : North);
        }

        // for all moves
        auto mask = (origin|target);
        Board[color] ^= (Board[piece] ^= mask, mask);

        if (piece == King) {
            auto idx = color == White ? 0:2;
            Board.castling_rights[idx] = Board.castling_rights[idx+1] = 0;
        }

        if (piece == Rooks) {
            if (color == White) {
                if (origin == h1) Board.castling_rights[0] = 0; // K
                if (origin == a1) Board.castling_rights[1] = 0; // Q
            } else {
                if (origin == h8) Board.castling_rights[2] = 0; // k
                if (origin == a8) Board.castling_rights[3] = 0; // q
            }
        }

        bool legal = true;
        auto king_square = Utils::IndexLS1B(Board[King] & Board[color]);
        // if (king_square != NoSquare) {
            if      (color == White) legal = !IsSquareAttacked<Black>(king_square);
            else if (color == Black) legal = !IsSquareAttacked<White>(king_square);
        // }
        Board.to_play = static_cast<EnumColor>(!Board.to_play);

        return legal;

    }


    [[nodiscard]] static constexpr auto DecodeFlags(const Move& move) noexcept
    -> std::array<EnumMoveFlags, 10> { auto flags = (move.encoded >> 12) & 0xf;
        return {
            static_cast<EnumMoveFlags>( flags == Quiet           ),
            static_cast<EnumMoveFlags>( flags == Capture         ),
            static_cast<EnumMoveFlags>( flags == DoublePawnPush  ),
            static_cast<EnumMoveFlags>( flags == CastleKing      ),
            static_cast<EnumMoveFlags>( flags == CastleQueen     ),
            static_cast<EnumMoveFlags>( flags == EnPassant       ),
            static_cast<EnumMoveFlags>( flags == KnightPromotion ),
            static_cast<EnumMoveFlags>( flags == BishopPromotion ),
            static_cast<EnumMoveFlags>( flags == RookPromotion   ),
            static_cast<EnumMoveFlags>( flags == QueenPromotion  )
        };
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


template <EnumColor Color, EnumPiece Piece>
inline auto GenerateMoves(std::array<Move, 218>::iterator& IT_MoveList) noexcept {
    auto occupancy        = (Board[Color] | Board[!Color]);
    auto set              = (Board[Color] & Board[ Piece]);

    while (set) {
        EnumSquare origin = Utils::PopLS1B(set);

        if constexpr(Piece == Pawns) {
            constexpr auto promotion_rank = (Color == White ? Rank_8 : Rank_1);
            constexpr auto starting_rank  = (Color == White ? Rank_2 : Rank_7);
            constexpr auto up             = (Color == White ? North  : South );

            EnumSquare target = origin + up;
            if ((target & promotion_rank) & ~occupancy) {
                *IT_MoveList++ = Move::Encode<Piece>(origin, target, KnightPromotion);
                *IT_MoveList++ = Move::Encode<Piece>(origin, target, BishopPromotion);
                *IT_MoveList++ = Move::Encode<Piece>(origin, target, RookPromotion  );
                *IT_MoveList++ = Move::Encode<Piece>(origin, target, QueenPromotion );
            }
            else {
                if (target & ~occupancy) {
                    *IT_MoveList++ = Move::Encode<Piece>(origin, target, Quiet);
                    if ((origin & starting_rank) && ((target+up) & ~occupancy))
                        *IT_MoveList++ = Move::Encode<Piece>(origin, (target+up), DoublePawnPush);
                }
            }

            auto attacks = (GetAttack<Color, Piece>::On(origin) & Board[!Color]);
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                *IT_MoveList++ = Move::Encode<Piece>(origin, attack, Capture);
                if (attack & promotion_rank) {
                    *(IT_MoveList++-1) = Move::Encode<Piece>(origin, attack, KnightPromotion | Capture);
                    *IT_MoveList++ = Move::Encode<Piece>(origin, attack, BishopPromotion | Capture);
                    *IT_MoveList++ = Move::Encode<Piece>(origin, attack, RookPromotion   | Capture);
                    *IT_MoveList++ = Move::Encode<Piece>(origin, attack, QueenPromotion  | Capture);
                }
            }

            if (Board.GetEnPassant()) {
                if (GetAttack<Color, Piece>::On(origin) & Board.GetEnPassant())
                    *IT_MoveList++ = Move::Encode<Piece>(origin, Board.GetEnPassant(), EnPassant);
            }
        }

        if constexpr(Piece == King || Piece == Knights) {
            auto attacks = GetAttack<Piece>::On(origin) & ~Board[Color];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                if (attack & ~Board[!Color])
                     *IT_MoveList++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *IT_MoveList++ = Move::Encode<Piece>(origin, attack, Capture);
            }

            if constexpr(Piece == King) {
                constexpr auto king = (Color == White ? e1:e8);

                constexpr auto Kk = (Color == White ? 0 : 2);
                if (Board.GetCastlingRights()[Kk]) {
                    if (!(((king+1) | (king+2)) & occupancy))
                        if (!IsSquareAttacked<EnumColor(!Color)>(king)
                         && !IsSquareAttacked<EnumColor(!Color)>(king+1)
                         && !IsSquareAttacked<EnumColor(!Color)>(king+2))
                            *IT_MoveList++ = Move::Encode<Piece>(origin, king+2, CastleKing);
                }

                constexpr auto Qq = (Color == White ? 1 : 3);
                if (Board.GetCastlingRights()[Qq]) {
                    if (!(((king-1) | (king-2) | (king-3)) & occupancy))
                        if (!IsSquareAttacked<EnumColor(!Color)>(king)
                         && !IsSquareAttacked<EnumColor(!Color)>(king-1)
                         && !IsSquareAttacked<EnumColor(!Color)>(king-2))
                            *IT_MoveList++ = Move::Encode<Piece>(origin, king-2, CastleQueen);
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
                     *IT_MoveList++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *IT_MoveList++ = Move::Encode<Piece>(origin, attack, Capture);
            }
        }
    }
}

static_assert(std::is_trivially_copyable<ChessBoard>::value == true, "Failed");

template <EnumColor Color>
inline auto GenerateAllMoves() noexcept {
    std::array<Move, 256> move_list { };
    auto iterator = move_list.begin();
    GenerateMoves<Color, Pawns>(iterator);
    GenerateMoves<Color, Knights>(iterator);
    GenerateMoves<Color, Bishops>(iterator);
    GenerateMoves<Color, Rooks>(iterator);
    GenerateMoves<Color, Queens>(iterator);
    GenerateMoves<Color, King>(iterator);
    return move_list;
}

inline std::uint64_t perft(int depth) {
    std::uint64_t nodes = 0;
    if (depth == 0) return 1ULL;

    std::array<Move, 256> MoveList;
    if (Board.to_play == White)
         MoveList = GenerateAllMoves<White>();
    else MoveList = GenerateAllMoves<Black>();

    auto backup = Board;
    for (auto move: MoveList) if (move.encoded) {
        if (!move.Make()) {
            // std::cout << move << "  !!!!! ILLEGAL !!!!!\n" << Board << '\n';
            // getchar();
            Board = backup; continue;
        }
        // std::cout << move << '\n' << Board << '\n';
        // getchar();
        nodes += perft(depth-1);
        // getchar();
        Board = backup;
    }
    return nodes;
}

#include <chrono>
int main(int argc, char* argv[]) {
    std::cout << Board << std::endl;
    auto started = std::chrono::high_resolution_clock::now();

    auto nodes = perft(std::atoi(argv[1]));

    auto done = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(done-started).count();
    std::uint64_t nps = nodes / (ms/1000.00f);

    std::cout.imbue(std::locale(""));
    std::cout << "nodes: " << nodes << std::endl
              << "nps  : " << nps   << std::endl
              << "ms   : " << ms    << std::endl;
    return 0;
}
