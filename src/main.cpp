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

template <EnumColor Color> [[nodiscard]] __attribute__((always_inline))
inline auto InCheck(ChessBoard& Board, EnumSquare square) noexcept {
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

struct alignas(4) Move final {
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

    template<EnumColor Color> [[nodiscard]] __attribute__((always_inline))
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
        }

        else { Board.en_passant = EnumSquare(0);
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

            else if (flags == DoublePush )
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
    template <EnumColor Color> __attribute__((always_inline))
    static inline auto Run(ChessBoard& Board) noexcept
    -> std::tuple<std::array<Move, 218>, std::uint8_t> {
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


    template <EnumColor Color, EnumPiece Piece> __attribute__((always_inline))
    static inline auto PseudoLegal(ChessBoard& Board, std::array<Move, 218>::iterator& Moves) noexcept {
        auto set       = (Board[Color] & Board[ Piece]);
        auto occupancy = (Board[Color] | Board[!Color]);

        while (set) {
        EnumSquare origin = Utils::PopLS1B(set);
        // EnumSquare origin = Utils::IndexLS1B(set);

        /////////////////////////////////////// PAWNS ////////////////////////////////////////

        if constexpr(Piece == Pawns) {
            constexpr auto StartingRank  = (Color == White ? Rank_2 : Rank_7);
            constexpr auto PromotionRank = (Color == White ? Rank_8 : Rank_1);
            constexpr auto Up            = (Color == White ? North  : South );

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

            auto attacks = GetAttack<Color, Piece>::On(origin) & Board[!Color];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                // auto attack = Utils::IndexLS1B(attacks);
                *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
                if (attack & PromotionRank) { --Moves;
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionKnight | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionBishop | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionRook   | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionQueen  | Capture);
                }
                // attacks ^= attack;
            }

            if (Board.en_passant)
                if (GetAttack<Color, Piece>::On(origin) & Board.en_passant)
                    *Moves++ = Move::Encode<Piece>(origin, Board.en_passant, EnPassant);
        }

        /////////////////////////////////// KNIGHTS / KING ///////////////////////////////////

        if constexpr(Piece == Knights || Piece == King) {
            auto attacks = GetAttack<Piece>::On(origin) & ~Board[Color];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                // auto attack = Utils::IndexLS1B(attacks); //Utils::PopLS1B(attacks);
                if (attack & ~Board[!Color])
                     *Moves++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
                // attacks ^= attack;
            }

            if constexpr(Piece == King) {
                constexpr auto king = (Color == White ? e1:e8);

                constexpr auto Kk = (Color == White ? 0 : 2);
                if (Board.castling_rights[Kk]) {
                    if (Board[Rooks] & king+3) {
                        if (!(((king+1) | (king+2)) & occupancy))
                            if (!InCheck<Color>(Board, king)
                            &&  !InCheck<Color>(Board, king+1)
                            &&  !InCheck<Color>(Board, king+2))
                                *Moves++ = Move::Encode<Piece>(origin, king+2, CastleKing);
                    } else Board.castling_rights[Kk] = 0;
                }

                constexpr auto Qq = (Color == White ? 1 : 3);
                if (Board.castling_rights[Qq]) {
                    if (Board[Rooks] & king-3) {
                        if (!(((king-1) | (king-2) | (king-3)) & occupancy))
                            if (!InCheck<Color>(Board, king)
                            &&  !InCheck<Color>(Board, king-1)
                            &&  !InCheck<Color>(Board, king-2))
                                *Moves++ = Move::Encode<Piece>(origin, king-2, CastleQueen);
                    } else Board.castling_rights[Qq] = 0;
                }
            }
        }

        ////////////////////////////// BISHOPS / ROOKS / QUEEN ///////////////////////////////

        if constexpr(Piece == Bishops || Piece == Rooks || Piece == Queens) {
            auto attacks = Bitboard(0);
            attacks =  GetAttack<Piece>::On(origin, occupancy) & ~Board[Color];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                // auto attack = Utils::IndexLS1B(set);
                if (attack & ~Board[!Color])
                     *Moves++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
                // attacks ^= attack;
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // set ^= origin;
        }
    }

    template <EnumColor Color>
    static std::uint64_t _Perft(ChessBoard& Board, int depth) noexcept {
        constexpr auto Other = ~Color;
        std::uint64_t nodes = 0;
        if (depth == 0) return 1ULL;

        ChessBoard Old = Board;
        auto [MoveList, nmoves] = MoveGen::Run<Color>(Board);
        if (depth == 1) return nmoves; // Pseudo-legal bulk counting?
        for (auto move = 0; move < nmoves; move++) {
            if (Move::Make<Color>(Board, MoveList[move]))
                nodes += _Perft<Other>(Board, depth-1);
            Board = Old;
        } return nodes;
    }

     MoveGen()=delete;
    ~MoveGen()=delete;
};


#define POSITION "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"


// std::array<std::uint64_t, 102400> RookAttacks { };
// std::array<std::uint64_t*,    64> RookOffset = {
//   &RookAttacks[0] + 86016, &RookAttacks[0] + 73728,
//   &RookAttacks[0] + 36864, &RookAttacks[0] + 43008,
//   &RookAttacks[0] + 47104, &RookAttacks[0] + 51200,
//   &RookAttacks[0] + 77824, &RookAttacks[0] + 94208,
//   &RookAttacks[0] + 69632, &RookAttacks[0] + 32768,
//   &RookAttacks[0] + 38912, &RookAttacks[0] + 10240,
//   &RookAttacks[0] + 14336, &RookAttacks[0] + 53248,
//   &RookAttacks[0] + 57344, &RookAttacks[0] + 81920,
//   &RookAttacks[0] + 24576, &RookAttacks[0] + 33792,
//   &RookAttacks[0] + 6144,  &RookAttacks[0] + 11264,
//   &RookAttacks[0] + 15360, &RookAttacks[0] + 18432,
//   &RookAttacks[0] + 58368, &RookAttacks[0] + 61440,
//   &RookAttacks[0] + 26624, &RookAttacks[0] + 4096,
//   &RookAttacks[0] + 7168,  &RookAttacks[0] + 0,
//   &RookAttacks[0] + 2048,  &RookAttacks[0] + 19456,
//   &RookAttacks[0] + 22528, &RookAttacks[0] + 63488,
//   &RookAttacks[0] + 28672, &RookAttacks[0] + 5120,
//   &RookAttacks[0] + 8192,  &RookAttacks[0] + 1024,
//   &RookAttacks[0] + 3072,  &RookAttacks[0] + 20480,
//   &RookAttacks[0] + 23552, &RookAttacks[0] + 65536,
//   &RookAttacks[0] + 30720, &RookAttacks[0] + 34816,
//   &RookAttacks[0] + 9216,  &RookAttacks[0] + 12288,
//   &RookAttacks[0] + 16384, &RookAttacks[0] + 21504,
//   &RookAttacks[0] + 59392, &RookAttacks[0] + 67584,
//   &RookAttacks[0] + 71680, &RookAttacks[0] + 35840,
//   &RookAttacks[0] + 39936, &RookAttacks[0] + 13312,
//   &RookAttacks[0] + 17408, &RookAttacks[0] + 54272,
//   &RookAttacks[0] + 60416, &RookAttacks[0] + 83968,
//   &RookAttacks[0] + 90112, &RookAttacks[0] + 75776,
//   &RookAttacks[0] + 40960, &RookAttacks[0] + 45056,
//   &RookAttacks[0] + 49152, &RookAttacks[0] + 55296,
//   &RookAttacks[0] + 79872, &RookAttacks[0] + 98304
// };

// const std::array<std::uint64_t, 64> RookMagics {
//     0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
//     0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
//     0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
//     0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
//     0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
//     0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
//     0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
//     0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
//     0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
//     0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
//     0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
//     0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
//     0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
//     0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
//     0x00fffcddfced714aULL, 0x007ffcddfced714aULL, 0x003fffcdffd88096ULL, 0x0000040810002101ULL,
//     0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001fffaabfad1a2ULL
// };

// const std::array<int, 64> RookShifts {
//     52, 53, 53, 53, 53, 53, 53, 52,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 54, 54, 54, 54, 53,
//     53, 54, 54, 53, 53, 53, 53, 53
// };

// [[nodiscard]] static _constexpr auto AttackTable() noexcept {
//     auto get_attack = [](EnumSquare square, Bitboard occupancy) constexpr {
//         Bitboard a = 0ULL, rk = 0ULL, o = occupancy;
//         int tr = square / 8, tf = square % 8;
//         #define SET_SQUARE { rk=0; rk|=EnumSquare(f+r*8); a|=rk; if (rk&o) break; }
//         for (int r = tr+1, f = tf;   r <= 7; r++) SET_SQUARE // N
//         for (int r = tr-1, f = tf;   r >= 0; r--) SET_SQUARE // S
//         for (int r = tr,   f = tf+1; f <= 7; f++) SET_SQUARE // E
//         for (int r = tr,   f = tf-1; f >= 0; f--) SET_SQUARE // W
//         #undef  SET_SQUARE
//         return a;
//     };

//     auto get_occupancy = [](int index, Bitboard attack_mask) constexpr {
//         Bitboard occupancy = 0ULL;
//         const auto mask_population = Utils::BitCount(attack_mask);
//         for (int count = 0; count < mask_population; count++) {
//             auto square = Utils::PopLS1B(attack_mask);
//             if (index & (1 << count))
//                 occupancy |= (1ULL << square);
//         } return occupancy;
//     };

//     std::array<Bitboard, 64> masks          = Generator::Attacks<Rooks>::MaskTable();
//     std::array<int,      64> masks_bitcount = Generator::Attacks<Rooks>::MaskTableBitCount();

//     for (EnumSquare square = a1; square <= h8; ++square) {

//         auto attack_mask   = masks[square];
//         auto relevant_bits = masks_bitcount[square];
//         int  permutations  = 1 << relevant_bits;

//         for (int index = 0; index < permutations; index++) {
//             auto occupancy = get_occupancy(index, attack_mask);
//             int magic_index = (occupancy * RookMagics[square]) >> RookShifts[square];
//             *(RookOffset[square] + magic_index) = get_attack(square, occupancy);
//             // std::cout << "\n\n";
//             // std::cout << square << magic_index << relevant_bits;
//             // Utils::Print(*(RookOffset[square] + magic_index));
//             // Utils::Print(get_attack(square, occupancy));
//         }
//     }
// }

// auto FancyMagics_SOA() {
//     struct Magic {
//         std::array<Bitboard*,     64> Attack;
//         std::array<Bitboard,      64> Mask;
//         std::array<std::uint64_t, 64> Number;
//         std::array<std::int32_t,  64> Shift;
//     };

//     return Magic {
//         .Attack = RookOffset,
//         .Mask    = Generator::Attacks<Rooks>::MaskTable(),
//         .Number  = RookMagics,
//         .Shift   = RookShifts,
//     };
// }

// auto Magic = FancyMagics_SOA();

// auto inline Get(EnumSquare square, Bitboard occupancy) {
//     occupancy  &= Magic.Mask  [square];
//     occupancy  *= Magic.Number[square];
//     occupancy >>= Magic.Shift [square];
//     return        Magic.Attack[square][occupancy];
// }

int main(int argc, char* argv[]) { (void)argc;

    ChessBoard Board(STARTING_POSITION);
    // std::cout  << Board << std::endl;

    // MoveGen::Perft(Board, std::atoi(argv[1]));
    AttackTable();
    auto square = a1;
    auto occupancy = (a5 | f1 | e4 | g2);
    Utils::Print(Get(a1, occupancy));
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
