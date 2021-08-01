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
        std::cout << "{" << SquareIndex[origin] << "} | " << (Color?"B":"W");

        if constexpr(Piece == Pawns) {
            std::cout << " | Pawns   | -> "<< "[ ";
            if constexpr(Color == White) {
                EnumSquare dest = origin+8;
                auto attacks = GetAttack<Color, Piece>::On(origin);
                attacks &= (~Board[Color] | Board[!Color]);
                while (attacks) {
                    EnumSquare attack = Utils::PopLS1B(attacks);
                    std::cout << SquareIndex[attack] << " ";
                }
                if ((dest & Rank_8) & ~(Board[Color] | Board[!Color])) {
                    std::cout << SquareIndex[dest] << "Q "; // promotion here
                    std::cout << SquareIndex[dest] << "R "; // promotion here
                    std::cout << SquareIndex[dest] << "B "; // promotion here
                    std::cout << SquareIndex[dest] << "N "; // promotion here
                } else {
                    if (dest & ~(Board[Color] | Board[!Color]))
                        std::cout << SquareIndex[dest] << ' '; // single
                    if ((origin & Rank_2)
                     && (dest+8 & ~(Board[Color] | Board[!Color])))
                        std::cout << SquareIndex[dest+8] << "D "; // double
                }
            }
            std::cout << ']';
        }

        if constexpr(Piece == Knights) {
            auto attacks = GetAttack<Piece>::On(origin);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | Knight  | -> "<< "[ ";
            while (attacks) {
                EnumSquare attack = Utils::PopLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
            }
            std::cout << ']';
        }

        if constexpr(Piece == Bishops) {
            auto occupancy = (Board[Color] | Board[!Color]);
            auto attacks = GetAttack<Piece>::On(origin, occupancy);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | Bishops | -> "<< "[ ";
            while (attacks) {
                EnumSquare attack = Utils::PopLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
            }
            std::cout << ']';
        }

        if constexpr(Piece == Rooks) {
            auto occupancy = (Board[Color] | Board[!Color]);
            auto attacks = GetAttack<Piece>::On(origin, occupancy);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | Rooks   | -> "<< "[ ";
            while (attacks) {
                EnumSquare attack = Utils::PopLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
            }
            std::cout << ']';
        }

        if constexpr(Piece == Queens) {
            auto occupancy = (Board[Color] | Board[!Color]);
            auto attacks = GetAttack<Bishops>::On(origin, occupancy)
                         | GetAttack<Rooks  >::On(origin, occupancy);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | Queens  | -> "<< "[ ";
            while (attacks) {
                EnumSquare attack = Utils::PopLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
            }
            std::cout << ']';
        }


        if constexpr(Piece == King) {
            auto attacks = GetAttack<King>::On(origin);
            attacks &= (~Board[Piece] | Board[!Color]);
            std::cout << " | King    | -> "<< "[ ";
            while (attacks) {
                EnumSquare attack = Utils::PopLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
            }
            std::cout << ']';
        }
        std::cout << "\n";
    }
}
#include <random>
int main() {
    std::cout << Board << std::endl;

    GenerateMoves<White, Pawns>();
    GenerateMoves<White, Knights>();
    GenerateMoves<White, Bishops>();
    GenerateMoves<White, Rooks>();
    GenerateMoves<White, Queens>();
    GenerateMoves<White, King>();

    return 0;
}
