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

#define POSITION "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
ChessBoard Board(STARTING_POSITION);

template <EnumColor Color>
[[nodiscard]] inline auto InCheck(EnumSquare square) noexcept {
    const auto enemies   = Board[~Color];
    const auto occupancy = Board[ Color] | enemies;
    return (
        GetAttack<Color, Pawns>::On(square           ) & (Board[Pawns  ] & enemies) ? true :
        GetAttack<Knights     >::On(square           ) & (Board[Knights] & enemies) ? true :
        GetAttack<Bishops     >::On(square, occupancy) & (Board[Bishops] & enemies) ? true :
        GetAttack<Rooks       >::On(square, occupancy) & (Board[Rooks  ] & enemies) ? true :
        GetAttack<Queens      >::On(square, occupancy) & (Board[Queens ] & enemies) ? true :
        false
    );
}

struct alignas(4) Move final {
    // std::uint16_t encoded;
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

    template<EnumColor Color> [[nodiscard]] __attribute__((always_inline))
    static auto Make(Move& move) noexcept {
        constexpr auto Allies    = Color, Enemies = ~Color;
        constexpr auto Down      = Allies == White ? South : North;
        constexpr auto KingRook  = Allies == White ? h1 : h8;
        constexpr auto QueenRook = Allies == White ? a1 : a8;
        constexpr auto Castle    = Allies == White ? (h1|f1) : (h8|f8);
        constexpr auto Kk        = Allies == White ? 0 : 2;
        constexpr auto Qq        = Allies == White ? 1 : 3;

        const auto [piece, origin, target, flags] = move;
        Board.to_play    = Enemies;
        Board.en_passant = EnumSquare(0);

        //////////////////////////////////////// QUIET ///////////////////////////////////////

        if (flags == Quiet) {
            auto move_mask = (origin|target);
            Board[Allies] ^= (Board[piece] ^= move_mask, move_mask);
            if (piece == King) {
                Board.castling_rights[Kk] = 0;
                Board.castling_rights[Qq] = 0;
            } else if (piece == Rooks) {
                if (origin == KingRook ) Board.castling_rights[Kk] = 0;
                if (origin == QueenRook) Board.castling_rights[Qq] = 0;
            } return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        }

        ////////////////////////////////////// CAPTURE ///////////////////////////////////////

        if (flags & Capture) {
            std::for_each(&Board[Pawns], &Board[King],
            [&, t=target](auto& set) { if (set & t) Board[Enemies] ^= (set ^= t, t); });
        }

        ////////////////////////////////////// SPECIAL ///////////////////////////////////////

        else if (flags == DoublePush )
            Board.en_passant = target + Down;

        else if (flags == CastleKing || flags == CastleQueen)
            Board[Allies] ^= (Board[Rooks] ^= Castle, Castle);

        if (flags == EnPassant)
            Board[Enemies] ^= (Board[Pawns] ^= target+Down, target+Down);

        if (flags & PromotionKnight) {
            const auto promotion =
            flags == PromotionBishop || flags == XPromotionBishop ? Bishops :
            flags == PromotionRook   || flags == XPromotionRook   ? Rooks   :
            flags == PromotionQueen  || flags == XPromotionQueen  ? Queens  :Knights;
            Board[promotion] |= target; Board[Pawns] ^= target;
        }

        if (piece == King) {
            Board.castling_rights[Kk] = 0;
            Board.castling_rights[Qq] = 0;
        } else if (piece == Rooks) {
            if (origin == KingRook ) Board.castling_rights[Kk] = 0;
            if (origin == QueenRook) Board.castling_rights[Qq] = 0;
        }

        //////////////////////////////////////////////////////////////////////////////////////

        const auto move_mask = (origin|target);
        Board[Allies] ^= (Board[piece] ^= move_mask, move_mask);
        return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
    }

    friend inline std::ostream& operator<<(std::ostream& os, const Move& move) {
        return os << std::left << std::setw(8) << move.piece << std::setw(0) << "| "
                  << move.origin << '-' << move.target << " | " << move.flags;
    }
};


class MoveGen final {
public:
    template <EnumColor Color> __attribute__((always_inline))
    static inline auto Run() noexcept -> std::tuple<std::array<Move, 218>, std::uint8_t> {
        std::array<Move, 218> moves;

        auto iterator = moves.begin();
        PseudoLegal<Color, Pawns   >(iterator);
        PseudoLegal<Color, Knights >(iterator);
        PseudoLegal<Color, Bishops >(iterator);
        PseudoLegal<Color, Rooks   >(iterator);
        PseudoLegal<Color, Queens  >(iterator);
        PseudoLegal<Color, King    >(iterator);

        const auto nmoves = std::distance(moves.begin(), iterator);
        return { moves, nmoves };
    }

    static std::uint64_t Perft(int depth) noexcept {
        std::uint64_t nodes = 0;

        auto started  = std::chrono::steady_clock::now();
        if (Board.to_play == White)
            nodes = _Perft<White>(depth);
        else if (Board.to_play == Black)
            nodes = _Perft<Black>(depth);
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

    template <EnumColor Color>
    static std::uint64_t _Perft(int depth) noexcept {
        constexpr auto Other = ~Color;
        if (depth == 0) return 1ULL;
        std::uint64_t nodes = 0;
        ChessBoard Old = Board;
        auto [MoveList, nmoves] = MoveGen::Run<Color>();
        for (auto move = 0; move < nmoves; move++) {
            if (Move::Make<Color>(MoveList[move]))
                nodes += _Perft<Other>(depth-1);
            Board = Old;
        } return nodes;
    }

    template <EnumColor Color, EnumPiece Piece> __attribute__((always_inline))
    static inline auto PseudoLegal(std::array<Move, 218>::iterator& Moves) noexcept {
        auto set       = (Board[Color] & Board[ Piece]);
        auto occupancy = (Board[Color] | Board[!Color]);

        while (set) {
        EnumSquare origin = Utils::PopLS1B(set);

        /////////////////////////////////////// PAWNS ////////////////////////////////////////

        if constexpr(Piece == Pawns) {
            constexpr auto RelativeUp    = (Color == White ? North  : South );
            constexpr auto StartingRank  = (Color == White ? Rank_2 : Rank_7);
            constexpr auto PromotionRank = (Color == White ? Rank_8 : Rank_1);

            auto empty     = ~occupancy;

            EnumSquare target = origin + RelativeUp;
            if ((target & PromotionRank) & empty) {
                *Moves++ = Move::Encode<Piece>(origin, target, PromotionKnight);
                *Moves++ = Move::Encode<Piece>(origin, target, PromotionBishop);
                *Moves++ = Move::Encode<Piece>(origin, target, PromotionRook  );
                *Moves++ = Move::Encode<Piece>(origin, target, PromotionQueen );
            } else if (target & empty) {
                *Moves++ = Move::Encode<Piece>(origin, target, Quiet);
                if ((origin & StartingRank) && ((target+RelativeUp) & empty))
                    *Moves++ = Move::Encode<Piece>(origin, (target+RelativeUp), DoublePush);
            }

            auto attacks = GetAttack<Color, Piece>::On(origin) & Board[!Color];
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
                if (GetAttack<Color, Piece>::On(origin) & Board.en_passant)
                    *Moves++ = Move::Encode<Piece>(origin, Board.en_passant, EnPassant);
        }

        /////////////////////////////////// KNIGHTS / KING ///////////////////////////////////

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
                if (Board.castling_rights[Kk]) {
                    if (Board[Rooks] & king+3) {
                        if (!(((king+1) | (king+2)) & occupancy))
                            if (!InCheck<Color>(king)
                            &&  !InCheck<Color>(king+1)
                            &&  !InCheck<Color>(king+2))
                                *Moves++ = Move::Encode<Piece>(origin, king+2, CastleKing);
                    } else Board.castling_rights[Kk] = 0;
                }

                constexpr auto Qq = (Color == White ? 1 : 3);
                if (Board.castling_rights[Qq]) {
                    if (Board[Rooks] & king-3) {
                        if (!(((king-1) | (king-2) | (king-3)) & occupancy))
                            if (!InCheck<Color>(king)
                            &&  !InCheck<Color>(king-1)
                            &&  !InCheck<Color>(king-2))
                                *Moves++ = Move::Encode<Piece>(origin, king-2, CastleQueen);
                    } else Board.castling_rights[Qq] = 0;
                }
            }
        }

        ////////////////////////////// BISHOPS / ROOKS / QUEEN ///////////////////////////////

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

        /////////////////////////////////////////////////////////////////////////////////////
        }
    }

     MoveGen()=delete;
    ~MoveGen()=delete;
};


int main(int argc, char* argv[]) { (void)argc;
    std::cout << Board << std::endl;
    MoveGen::Perft(std::atoi(argv[1]));
    return 0;
}












// #define MoveUpdate()
        //     auto move_mask = (origin|target);
        //     Board[Allies] ^= (Board[piece] ^= move_mask, move_mask);

        // #define CaptureUpdate()
        //     std::for_each(&Board[Pawns], &Board[King],
        //     [&, t=target](auto& set) { if (set & t) Board[Enemies] ^= (set ^= t, t); });

        // constexpr auto QueenRook = Allies == White ? a1 : a8;
        // constexpr auto KingRook  = Allies == White ? h1 : h8;
        // constexpr auto Kk        = Allies == White ? 0 : 2;
        // constexpr auto Qq        = Allies == White ? 1 : 3;

        // switch (flags) {
        // case Quiet: {
        //     MoveUpdate();
        //     if      (piece == King) {
        //         Board.castling_rights[Kk] = 0;
        //         Board.castling_rights[Qq] = 0;
        //     }
        //     else if (piece == Rooks) {
        //         if      (origin == KingRook ) Board.castling_rights[Kk] = 0;
        //         else if (origin == QueenRook) Board.castling_rights[Qq] = 0;
        //     } return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case DoublePush: {
        //     MoveUpdate();
        //     Board.en_passant = target+Down;
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case CastleKing: [[fallthrough]];
        // case CastleQueen: {
        //     MoveUpdate();
        //     Board[Allies] ^= (Board[Rooks] ^= Castle, Castle);
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case Capture: {
        //     CaptureUpdate();
        //     MoveUpdate();
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }


        // case EnPassant: {
        //     MoveUpdate();
        //     Board[Enemies] ^= (Board[Pawns] ^= target+Down, target+Down);
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case PromotionKnight: {
        //     Board[Knights] |= target; Board[Pawns] ^= target;
        //     MoveUpdate();
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case PromotionBishop: {
        //     Board[Bishops] |= target; Board[Pawns] ^= target;
        //     MoveUpdate();
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case PromotionRook: {
        //     Board[Rooks] |= target; Board[Pawns] ^= target;
        //     MoveUpdate();
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case PromotionQueen: {
        //     Board[Queens] |= target; Board[Pawns] ^= target;
        //     MoveUpdate();
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case XPromotionKnight: {
        //     Board[Knights] |= target; Board[Pawns] ^= target;
        //     CaptureUpdate();
        //     MoveUpdate();
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case XPromotionBishop: {
        //     Board[Bishops] |= target; Board[Pawns] ^= target;
        //     CaptureUpdate();
        //     MoveUpdate();
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case XPromotionRook: {
        //     Board[Rooks] |= target; Board[Pawns] ^= target;
        //     CaptureUpdate();
        //     MoveUpdate();
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }

        // case XPromotionQueen: {
        //     Board[Queens] |= target; Board[Pawns] ^= target;
        //     CaptureUpdate();
        //     MoveUpdate();
        //     return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        // }
        // }

        // } return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
