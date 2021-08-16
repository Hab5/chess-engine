#include "ChessEngine.hpp"
#include "ChessBoard.hpp"
#include "GetAttack.hpp"
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
#include <chrono>
#include <cstring>

#define LIKELY(EXPR) __builtin_expect((bool)(EXPR), true)
#define UNLIKELY(EXPR) __builtin_expect((bool)(EXPR), false)

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

constexpr inline EnumMoveFlags operator+(EnumMoveFlags lhs, EnumMoveFlags rhs) noexcept {
    return static_cast<EnumMoveFlags>(static_cast<int>(lhs)+static_cast<int>(rhs));
}

constexpr inline EnumMoveFlags operator|(EnumMoveFlags lhs, EnumMoveFlags rhs) noexcept {
    return lhs+rhs;
}

template <EnumColor Color> [[nodiscard]] //__attribute__((always_inline))
static inline auto InCheck(ChessBoard& Board, EnumSquare square) noexcept {
    const auto enemies   = Board[~Color];
    const auto occupancy = Board[ Color] | enemies;

    #define Pp    (Board[Pawns  ]                  & enemies)
    #define Nn    (Board[Knights]                  & enemies)
    #define BbQq ((Board[Bishops] | Board[Queens]) & enemies)
    #define RrQq ((Board[Rooks  ] | Board[Queens]) & enemies)

    if      (GetAttack<Color, Pawns>::On(square)            & Pp  ) return true;
    else if (GetAttack<Knights     >::On(square)            & Nn  ) return true;
    else if (GetAttack<Bishops     >::On(square, occupancy) & BbQq) return true;
    else if (GetAttack<Rooks       >::On(square, occupancy) & RrQq) return true;
    else return false;
}

struct Move final {
    EnumPiece     piece;
    EnumSquare    origin;
    EnumSquare    target;
    EnumMoveFlags flags;

    template <EnumPiece Piece>
    [[nodiscard]] static inline constexpr auto Encode
    (EnumSquare origin, EnumSquare target, EnumMoveFlags flags) noexcept {
        return std::move(Move {
            .piece  = Piece,
            .origin = origin,
            .target = target,
            .flags  = flags
        });
    }
    
    template<EnumColor Color> [[nodiscard]] //__attribute__((always_inline))
    static inline auto Make(ChessBoard& Board, Move& move) noexcept {
        constexpr auto Allies    = Color, Enemies = ~Color;
        constexpr auto Down      = Allies == White ? South : North;
        constexpr auto KingRook  = Allies == White ? h1 : h8;
        constexpr auto QueenRook = Allies == White ? a1 : a8;
        constexpr auto Castle    = Allies == White ? (h1|f1) : (h8|f8);
        constexpr auto Kk        = Allies == White ? 0 : 2;
        constexpr auto Qq        = Allies == White ? 1 : 3;

        const auto [piece, origin, target, flags] = move;

        //////////////////////////////////////// QUIET ///////////////////////////////////////

        if (flags == Quiet) {
            Board.to_play    = Enemies;
            Board.en_passant = EnumSquare(0);
            Board[Allies] ^= (Board[piece] ^= (origin|target), (origin|target));
            if (piece == King) {
                Board.castling_rights[Kk] = 0;
                Board.castling_rights[Qq] = 0;
            } else if (piece == Rooks) {
                if (origin == KingRook ) Board.castling_rights[Kk] = 0;
                if (origin == QueenRook) Board.castling_rights[Qq] = 0;
            }
            return !InCheck<Allies>(Board, Utils::IndexLS1B(Board[King] & Board[Allies]));
        } else { Board.en_passant = EnumSquare(0);

         ////////////////////////////////////// CAPTURE ///////////////////////////////////////

            if (flags & Capture) {
                for (auto set = &Board[Pawns]; set != &Board[King]; ++set) {
                    if (*set & target) {
                        Board[Enemies] ^= (*set ^= target, target);
                        // break;
                    }
                }
                // std::for_each(&Board[Pawns], &Board[King],
                // [&, t=target](auto& set) { if (set & t) Board[Enemies] ^= (set ^= t, t); });
            }

        ////////////////////////////////////// SPECIAL ///////////////////////////////////////

            else if (flags == DoublePush)
                Board.en_passant = target + Down;

            else if (flags == CastleKing || flags == CastleQueen)
                Board[Allies] ^= (Board[Rooks] ^= Castle, Castle);

            if (flags == EnPassant)
                Board[Enemies] ^= (Board[Pawns] ^= target+Down, target+Down);

            else if (flags & PromotionKnight) {
                const auto promotion =
                flags == PromotionBishop || flags == XPromotionBishop ? Bishops :
                flags == PromotionRook   || flags == XPromotionRook   ? Rooks   :
                flags == PromotionQueen  || flags == XPromotionQueen  ? Queens  :Knights;
                Board[promotion] |= target; Board[Pawns] ^= target;
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
            return !InCheck<Allies>(Board, Utils::IndexLS1B(Board[King] & Board[Allies]));
        }
    }

    friend inline std::ostream& operator<<(std::ostream& os, const Move& move) {
        return os << std::left << std::setw(8) << move.piece << std::setw(0) << "| "
                  << move.origin << '-' << move.target << " | " << move.flags;
    }
};


class MoveGen final {
public:
    template <EnumColor Color> //__attribute__((always_inline))
    static inline auto Run(ChessBoard& Board) noexcept
    -> std::tuple<std::array<Move, 218>, int> {
        std::array<Move, 218> moves;

        auto iterator = moves.begin();
        PseudoLegal<Color, Pawns  >(Board, iterator);
        PseudoLegal<Color, Knights>(Board, iterator);
        PseudoLegal<Color, Bishops>(Board, iterator);
        PseudoLegal<Color, Rooks  >(Board, iterator);
        PseudoLegal<Color, Queens >(Board, iterator);
        PseudoLegal<Color, King   >(Board, iterator);

        const auto nmoves = std::distance(moves.begin(), iterator);
        return { std::move(moves), nmoves };
    }

    static std::uint64_t Perft(ChessBoard& Board, int depth) noexcept {
        std::uint64_t nodes = 0;

        auto started  = std::chrono::steady_clock::now();

        if (Board.to_play == White)
            nodes = _Perft<White>(Board, depth);
        else if (Board.to_play == Black)
            nodes = _Perft<Black>(Board, depth);

        auto finished = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast
                 <std::chrono::milliseconds>
                 (finished-started).count();
        
        auto sec = ms / 1000.00000f;
        std::cout.imbue(std::locale(""));
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "[depth=" << depth << "][" << nodes << "]["
            << "" << std::setprecision(2) << nodes/sec/1000000 << "Mnps]\n";

        return nodes;
    }


private:


    template <EnumColor Color, EnumPiece Piece> //__attribute__((always_inline))
    static inline auto PseudoLegal(ChessBoard& Board, std::array<Move, 218>::iterator& Moves) noexcept {
        constexpr auto Allies = Color, Enemies = ~Allies;
        auto set       = (Board[Allies] & Board[ Piece]);
        auto occupancy = (Board[Allies] | Board[Enemies]);

        while (set) {
        EnumSquare origin = Utils::PopLS1B(set);

        /////////////////////////////////////// PAWNS ////////////////////////////////////////

        if constexpr(Piece == Pawns) {
            constexpr auto StartingRank  = (Allies == White ? Rank_2 : Rank_7);
            constexpr auto PromotionRank = (Allies == White ? Rank_8 : Rank_1);
            constexpr auto Up            = (Allies == White ? North  : South );

            auto empty = ~occupancy;
            EnumSquare target = origin + Up;
            if (target & empty) {
                if (target & ~PromotionRank) {
                    *Moves++ = Move::Encode<Piece>(origin, target, Quiet);
                    if ((origin & StartingRank) && ((target+Up) & empty))
                        *Moves++ = Move::Encode<Piece>(origin, (target+Up), DoublePush);
                } else {
                    *Moves++ = Move::Encode<Piece>(origin, target, PromotionKnight);
                    *Moves++ = Move::Encode<Piece>(origin, target, PromotionBishop);
                    *Moves++ = Move::Encode<Piece>(origin, target, PromotionRook  );
                    *Moves++ = Move::Encode<Piece>(origin, target, PromotionQueen );
                }
            }

            auto attacks = GetAttack<Allies, Piece>::On(origin) & Board[Enemies];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
                if (attack & PromotionRank) { --Moves;
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionKnight | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionBishop | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionRook   | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionQueen  | Capture);
                }
            }

            if (Board.en_passant)
                if (GetAttack<Allies, Piece>::On(origin) & Board.en_passant)
                    *Moves++ = Move::Encode<Piece>(origin, Board.en_passant, EnPassant);
        }

        /////////////////////////////////// KNIGHTS / KING ///////////////////////////////////

        if constexpr(Piece == Knights || Piece == King) {
            auto attacks = GetAttack<Piece>::On(origin) & ~Board[Allies];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                if (attack & ~Board[Enemies])
                     *Moves++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
            }

            if constexpr(Piece == King) {
                constexpr auto king = (Allies == White ? e1:e8);

                constexpr auto Kk = (Allies == White ? 0 : 2);
                if (Board.castling_rights[Kk]) {
                    if (Board[Rooks] & king+3) {
                        if (!(((king+1) | (king+2)) & occupancy))
                            if (!InCheck<Allies>(Board, king)
                            &&  !InCheck<Allies>(Board, king+1)
                            &&  !InCheck<Allies>(Board, king+2))
                                *Moves++ = Move::Encode<Piece>(origin, king+2, CastleKing);
                    } else Board.castling_rights[Kk] = 0;
                }

                constexpr auto Qq = (Allies == White ? 1 : 3);
                if (Board.castling_rights[Qq]) {
                    if (Board[Rooks] & king-3) {
                        if (!(((king-1) | (king-2) | (king-3)) & occupancy))
                            if (!InCheck<Allies>(Board, king)
                            &&  !InCheck<Allies>(Board, king-1)
                            &&  !InCheck<Allies>(Board, king-2))
                                *Moves++ = Move::Encode<Piece>(origin, king-2, CastleQueen);
                    } else Board.castling_rights[Qq] = 0;
                }
            }
        }

        ////////////////////////////// BISHOPS / ROOKS / QUEEN ///////////////////////////////

        if constexpr(Piece == Bishops || Piece == Rooks || Piece == Queens) {
            auto attacks = Bitboard(0);
            attacks =  GetAttack<Piece>::On(origin, occupancy) & ~Board[Allies];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                if (attack & ~Board[Enemies])
                     *Moves++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////
        }
    }

    template <EnumColor Color> 
    static std::uint64_t _Perft(ChessBoard& Board, int depth) noexcept {
        constexpr auto Other = ~Color;
        std::uint64_t nodes = 0;
        if (depth == 0) return 1ULL;

        ChessBoard Old = Board;
        auto [MoveList, nmoves] = MoveGen::Run<Color>(Board);
        // if (depth == 1) return nmoves; // Pseudo-legal bulk counting?
        for (auto move = 0; move < nmoves; move++) {
            if (Move::Make<Color>(Board, MoveList[move]))
                nodes += _Perft<Other>(Board, depth-1);
            Board = Old;
        } return nodes;
    }

     MoveGen()=delete;
    ~MoveGen()=delete;
};


#define POSITION "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

int main(int argc, char* argv[]) { (void)argc;

    ChessBoard Board(STARTING_POSITION);
    // std::cout  << Board << std::endl;

    MoveGen::Perft(Board, std::atoi(argv[1]));

    // MoveGen::Perft(Board, 1);
    // MoveGen::Perft(Board, 2);
    // MoveGen::Perft(Board, 3);
    // MoveGen::Perft(Board, 4);
    // MoveGen::Perft(Board, 5);
    // MoveGen::Perft(Board, 6);
    // MoveGen::Perft(Board, 7);

    return 0;
}
