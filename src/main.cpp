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
bool IsSquareAttacked(int square) {
    constexpr auto Other = EnumColor(!Color);
    auto occ = Board[White] | Board[Black];
    return (
        Attack<Other, Pawns> ::On(square)      & (Board[Pawns  ] & Board[Color]) ? true :
        Attack<Knights     > ::On(square)      & (Board[Knights] & Board[Color]) ? true :
        Attack<Bishops     > ::On(square, occ) & (Board[Bishops] & Board[Color]) ? true :
        Attack<Rooks       > ::On(square, occ) & (Board[Rooks  ] & Board[Color]) ? true :
        Attack<Queens      > ::On(square, occ) & (Board[Queens ] & Board[Color]) ? true :
        Attack<King        > ::On(square)      & (Board[King   ] & Board[Color]) ? true :
        false
    );
}

template <EnumColor Color>
std::uint64_t GetAttackedSquares() {
    std::uint64_t attacked = 0ULL;
    for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++)
        if (IsSquareAttacked<Color>(square))
            attacked |= Utils::SetSquare(attacked, square);
    return attacked;
}

template <EnumColor Color, EnumPiece Piece>
auto GenerateMoves() {
    auto set = (Board[Piece] & Board[Color]);
    while (set) {
        auto origin = Utils::IndexLS1B(set);
        std::cout << "{" << SquareIndex[origin] << "} | " << (Color?"B":"W");

        if constexpr(Piece == Pawns) {
            std::cout << " | Pawns   | -> "<< "[ ";
            if constexpr(Color == White) {
                auto dest = origin+8;
                auto attacks = Attack<Color, Piece>::On(origin);
                attacks &= (~Board[Color] | Board[!Color]);
                while (attacks) {
                    auto attack = Utils::IndexLS1B(attacks);
                    std::cout << SquareIndex[attack] << " ";
                    attacks = Utils::PopSquare(attacks, attack);
                }
                if ((Utils::MakeSquare(dest) & Rank_8) & ~(Board[Color] | Board[!Color])) {
                    std::cout << SquareIndex[dest] << "Q "; // promotion here
                    std::cout << SquareIndex[dest] << "R "; // promotion here
                    std::cout << SquareIndex[dest] << "B "; // promotion here
                    std::cout << SquareIndex[dest] << "N "; // promotion here
                } else {
                    if (Utils::MakeSquare(dest) & ~(Board[Color] | Board[!Color]))
                        std::cout << SquareIndex[dest] << ' '; // single
                    if ((Utils::MakeSquare(origin) & Rank_2)
                     && (Utils::MakeSquare(dest+8) & ~(Board[Color] | Board[!Color])))
                        std::cout << SquareIndex[dest+8] << "D "; // double
                }
            }
            std::cout << ']';
        }

        if constexpr(Piece == Knights) {
            auto attacks = Attack<Piece>::On(origin);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | Knight  | -> "<< "[ ";
            while (attacks) {
                auto attack = Utils::IndexLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
                attacks = Utils::PopSquare(attacks, attack);
            }
            std::cout << ']';
        }

        if constexpr(Piece == Bishops) {
            auto occupancy = (Board[Color] | Board[!Color]);
            auto attacks = Attack<Piece>::On(origin, occupancy);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | Bishops | -> "<< "[ ";
            while (attacks) {
                auto attack = Utils::IndexLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
                attacks = Utils::PopSquare(attacks, attack);
            }
            std::cout << ']';
        }

        if constexpr(Piece == Rooks) {
            auto occupancy = (Board[Color] | Board[!Color]);
            auto attacks = Attack<Piece>::On(origin, occupancy);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | Rooks   | -> "<< "[ ";
            while (attacks) {
                auto attack = Utils::IndexLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
                attacks = Utils::PopSquare(attacks, attack);
            }
            std::cout << ']';
        }

        if constexpr(Piece == Queens) {
            auto occupancy = (Board[Color] | Board[!Color]);
            auto attacks = Attack<Bishops>::On(origin, occupancy)
                         | Attack<Rooks  >::On(origin, occupancy);
            attacks &= (~Board[Color] | Board[!Color]);
            std::cout << " | Queens  | -> "<< "[ ";
            while (attacks) {
                auto attack = Utils::IndexLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
                attacks = Utils::PopSquare(attacks, attack);
            }
            std::cout << ']';
        }


        if constexpr(Piece == King) {
            auto attacks = Attack<King>::On(origin);
            attacks &= (~Board[Piece] | Board[!Color]);
            std::cout << " | King    | -> "<< "[ ";
            while (attacks) {
                auto attack = Utils::IndexLS1B(attacks);
                std::cout << SquareIndex[attack] << " ";
                attacks = Utils::PopSquare(attacks, attack);
            }
            std::cout << ']';
        }

        set = Utils::PopSquare(set, origin);
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
    std::cout << '\n';
    GenerateMoves<Black, Knights>();
    GenerateMoves<Black, Bishops>();
    GenerateMoves<Black, Rooks>();
    GenerateMoves<Black, Queens>();
    GenerateMoves<Black, King>();
    return 0;
}
