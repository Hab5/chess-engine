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
        // .encoded = static_cast<std::uint16_t>((
        //            (( flags  & 0xf ) << 12)
        //          | ((+origin & 0x3f) << 6 )
        //          | ((+target & 0x3f) << 0 ))),
            .piece   = Piece,
            .origin = origin,
            .target = target,
            .flags  = flags
        };
    }

    // [[nodiscard]] static constexpr auto Decode(const Move& move) noexcept
    // -> std::tuple<EnumPiece, EnumSquare, EnumSquare, EnumMoveFlags> {
    //     return { move.piece,
    //         static_cast<EnumSquare   >((move.encoded >> 6 ) & 0x3f), // origin
    //         static_cast<EnumSquare   >((move.encoded >> 0 ) & 0x3f), // target
    //         static_cast<EnumMoveFlags>((move.encoded >> 12) & 0xf)   // flags
    //     };
    // }
    
    template<EnumColor Color> [[nodiscard]] __attribute__((always_inline))
    static auto Make(Move& move) noexcept {
        constexpr auto Allies = Color, Enemies = ~Color;
        constexpr auto Castle = Allies == White ? (h1|f1) : (h8|f8);
        constexpr auto Down = Allies == White ? South : North;

        const auto [piece, origin, target, flags] = move;//Move::Decode(move);

        Board.to_play    = Enemies;
        Board.en_passant = EnumSquare(0);

        // switch (flags) {
        // case Quiet:
        //     auto move_mask = (origin|target);
        //     Board[Allies] ^= (Board[piece] ^= move_mask, move_mask);
        //     if (piece == King) {
        //         constexpr auto CastlingRightsIdx = Allies == White ? 0 : 2;
        //         Board.castling_rights[CastlingRightsIdx]   = 0;
        //         Board.castling_rights[CastlingRightsIdx+1] = 0;
        //     } else if (piece == Rooks) {

        //     }
        // }

        if (flags == Quiet) {
            auto move_mask = (origin|target);
            Board[Allies] ^= (Board[piece] ^= move_mask, move_mask);
            if (piece == King) {
                constexpr auto CastlingRightsIdx = Allies == White ? 0 : 2;
                Board.castling_rights[CastlingRightsIdx]   = 0;
                Board.castling_rights[CastlingRightsIdx+1] = 0;
            } else if (piece == Rooks) {
                if constexpr (Allies == White) {
                    if (origin == h1) Board.castling_rights[0] = 0; // K
                    if (origin == a1) Board.castling_rights[1] = 0; // Q
                } else {
                    if (origin == h8) Board.castling_rights[2] = 0; // k
                    if (origin == a8) Board.castling_rights[3] = 0; // q
                }
            }
            return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
        }
        else if (flags & Capture) std::for_each(&Board[Pawns], &Board[King],
            [&, t=target](auto& set) { if (set & t) Board[Enemies] ^= (set ^= t, t); });
        else if (flags == DoublePush ) Board.en_passant = target + Down;
        else if (flags == CastleKing ) {
            Board[Allies] ^= (Board[Rooks] ^= Castle, Castle);
        }
        else if (flags == CastleQueen) {
            Board[Allies] ^= (Board[Rooks] ^= Castle, Castle);
        }
        if (flags == EnPassant) Board[Enemies] ^= (Board[Pawns] ^= target+Down, target+Down);
        if (flags & PromotionKnight) {
            const auto promotion =
                flags == PromotionBishop || flags == XPromotionBishop ? Bishops :
                flags == PromotionRook   || flags == XPromotionRook   ? Rooks   :
                flags == PromotionQueen  || flags == XPromotionQueen  ? Queens  : Knights;
            Board[promotion] |= target; Board[Pawns] ^= target;
        } if (piece == King) { // limit this ?
            constexpr auto CastlingRightsIdx = Allies == White ? 0 : 2;
            Board.castling_rights[CastlingRightsIdx]   = 0;
            Board.castling_rights[CastlingRightsIdx+1] = 0;
        } if (piece == Rooks) { // and this ?
            if constexpr (Allies == White) {
                if (origin == h1) Board.castling_rights[0] = 0; // K
                if (origin == a1) Board.castling_rights[1] = 0; // Q
            } else {
                if (origin == h8) Board.castling_rights[2] = 0; // k
                if (origin == a8) Board.castling_rights[3] = 0; // q
            }
        }

        auto move_mask = (origin|target);
        Board[Allies] ^= (Board[piece] ^= move_mask, move_mask);

        return !InCheck<Allies>(Utils::IndexLS1B(Board[King] & Board[Allies]));
    }

    // friend inline std::ostream& operator<<(std::ostream& os, const Move& move) {
    //     auto [piece, origin, target, flags] = Move::Decode(move);

    //     return os << std::left << std::setw(8) << piece << std::setw(0) << "| "
    //               << origin << '-' << target << " | " << flags;
    // }

    // friend inline bool operator>(const Move& move, const Move& other) {
    //     return move.encoded > other.encoded;
    // }
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

        auto nmoves = std::distance(moves.begin(), iterator);

        return { moves, nmoves };
    }

    template <EnumColor Color>
    static std::uint64_t Perft(int depth) noexcept {
        constexpr auto Other = ~Color;
        if (depth == 0) return 1ULL;
        std::uint64_t nodes = 0;
        ChessBoard Old = Board;
        auto [MoveList, nmoves] = MoveGen::Run<Color>();
        for (auto move = 0; move < nmoves; move++) {
            if (Move::Make<Color>(MoveList[move]))
                nodes += Perft<Other>(depth-1);
            Board = Old;
        } return nodes;
    }

private:
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
