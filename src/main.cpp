#include "ChessEngine.hpp"
#include "ChessBoard.hpp"
#include "Attack.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#include <iostream>
#include <vector>
#include <array>

#define POSITION "r3k2r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPpP/R3K2R b KQkq a3 0 1"
ChessBoard Board(POSITION);

template <EnumColor Color>
bool IsSquareAttacked(EnumSquare square) {
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
std::uint64_t GetAttackedSquares() {
    std::uint64_t attacked = 0ULL;
    for (EnumSquare square = EnumSquare::a1; square <= EnumSquare::h8; ++square)
        if (IsSquareAttacked<Color>(square))
            attacked |= square;
    return attacked;
}

template <EnumColor Color, EnumPiece Piece>
auto GenerateMoves() {
    auto set = (Board[Color] & Board[Piece]);

    while (set) {
        EnumSquare origin = Utils::PopLS1B(set);
        std::cout << "{" << origin << "} | " << Color;

        auto nothing_blocking = ~(Board[Color] | Board[!Color]);

        if constexpr(Piece == Pawns) {
            std::cout << " | " << Piece << " | -> "<< "[ ";

            constexpr auto promotion_rank = (Color == White ? Rank_8 : Rank_1);
            constexpr auto starting_rank  = (Color == White ? Rank_2 : Rank_7);
            constexpr auto up             = (Color == White ? North  : South );


            // QUIET
            EnumSquare dest = origin + up;
            if ((dest & promotion_rank) & nothing_blocking)
                std::cout << origin << dest << "* "; // promotion here
            else {
                if (dest & nothing_blocking) { // single
                    std::cout << origin << dest << ' ';
                    if ((origin & starting_rank) && ((dest + up) & nothing_blocking)) // double
                        std::cout << origin << (dest + up) << " ";
                }
            }

            // CAPTURES
            auto attacks = (GetAttack<Color, Piece>::On(origin) & Board[!Color]);
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                std::cout << origin << attack
                    << "%" << (attack & promotion_rank ? "* " : " "); // cap || prom_cap
            }

            // EN PASSANT
            if (Board.GetEnPassant()) {
                if (GetAttack<Color, Piece>::On(origin) & Board.GetEnPassant())
                    std::cout << origin << Board.GetEnPassant() << "& ";
            }

            std::cout << ']';

        }

        // else if constexpr(Piece == Knights || Piece == King) {
        //     auto attacks = GetAttack<Piece>::On(origin);
        //     attacks &= nothing_blocking;
        //     std::cout << " | " << Piece << " | -> "<< "[ ";
        //     while (attacks)
        //         std::cout << Utils::PopLS1B(attacks) << " ";
        //     std::cout << ']';
        // }
        std::cout << "\n";
    }
}

int main() {
    std::cout << Board << std::endl;

    GenerateMoves<Black, Pawns>();
    // GenerateMoves<White, Knights>();
    // GenerateMoves<White, Bishops>();
    // GenerateMoves<White, Rooks>();
    // GenerateMoves<White, Queens>();
    // GenerateMoves<White, King>();


    // for (EnumSquare square = a1; square <= h8; ++square) {
    //     Utils::Print(GetAttack<Queens>::On(square, 0ULL));
    //     std::getchar();
    // }
    return 0;
}
