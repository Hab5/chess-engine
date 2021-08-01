#include "ChessEngine.hpp"
#include "ChessBoard.hpp"
#include "Attack.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#include <iostream>
#include <vector>
#include <array>

#define POSITION "1nbqkbnr/Pppppppp/3NBR2/8/3K2P1/P1P1P3/5PPP/RNBQKBNR w KQkq - 0 1"
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
    auto set = (Board[Piece] & Board[Color]);
    while (set) {
        EnumSquare origin = Utils::PopLS1B(set);
        std::cout << "{" << origin << "} | " << Color;

        if constexpr(Piece == Pawns) {
            std::cout << " | " << Piece << " | -> "<< "[ ";
            auto nothing_blocking = ~(Board[Color] & Board[!Color]);
            if constexpr(Color == White) {
                EnumSquare dest = origin+8;
                auto attacks = GetAttack<Color, Piece>::On(origin);
                attacks &= (~Board[Color] | Board[!Color]);
                while (attacks)
                    std::cout << SquareStr[Utils::PopLS1B(attacks)] << " ";
                if ((dest & Rank_8) & nothing_blocking) {
                    std::cout << SquareStr[dest] << "Q "; // promotion here
                    std::cout << SquareStr[dest] << "R "; // promotion here
                    std::cout << SquareStr[dest] << "B "; // promotion here
                    std::cout << SquareStr[dest] << "N "; // promotion here
                } else {
                    if (dest & nothing_blocking)
                        std::cout << dest << ' '; // single
                    if ((origin & Rank_2) && (dest+8 & nothing_blocking))
                        std::cout << dest+8 << "D "; // double
                }
            }
            std::cout << ']';
        }

        else if constexpr(Piece == Knights || Piece == King) {
            auto attacks = GetAttack<Piece>::On(origin);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | " << Piece << " | -> "<< "[ ";
            while (attacks)
                std::cout << Utils::PopLS1B(attacks) << " ";
            std::cout << ']';
        }

        else if constexpr(Piece == Bishops || Piece == Rooks || Piece == Queens) {
            Bitboard attacks; auto occupancy = (Board[Color] | Board[!Color]);
            if (Piece == Queens)
                attacks = GetAttack<Bishops>::On(origin, occupancy)
                        | GetAttack<Rooks  >::On(origin, occupancy);
            else if (Piece == Bishops || Piece == Rooks)
                attacks = GetAttack<Piece>::On(origin, occupancy);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | " << Piece << " | -> " << "[ ";
            while (attacks)
                std::cout << Utils::PopLS1B(attacks) << " ";
            std::cout << ']';
        }
        std::cout << "\n";
    }
}

int main() {
    std::cout << Board << std::endl;

    GenerateMoves<White, Pawns>();
    GenerateMoves<White, Knights>();
    GenerateMoves<White, Bishops>();
    GenerateMoves<White, Rooks>();
    GenerateMoves<White, Queens>();
    GenerateMoves<White, King>();

    Utils::Print(GetAttack<Bishops>::On(e4, Bitboard(0) | g2 | d5 | b1));
    Utils::Print(GetAttack<Rooks>::On(e4, Bitboard(0) | e2 | a4 | f4 | e7));


    return 0;
}
