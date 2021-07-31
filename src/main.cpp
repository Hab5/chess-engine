#include "ChessEngine.hpp"
#include "ChessBoard.hpp"
#include "Attack.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#include <iostream>
#include <vector>
#include <array>




#define POSITION "8/8/4r3/3B4/2R5/8/8/8 w - -"
ChessBoard Board(STARTING_POSITION);

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

    // if (Attack<EnumColor(!Color), Pawns>::On(square) & (Board[Pawns] & Board[Color])) return true;
    // if (Attack<Knights>::On(square) & (Board[Knights] & Board[Color])) return true;
    // if (Attack<Bishops>::On(square, (Board[White] | Board[Black])) & (Board[Bishops] & Board[Color])) return true;
    // if (Attack<Rooks>  ::On(square, (Board[White] | Board[Black])) & (Board[Rooks]   & Board[Color])) return true;
    // if (Attack<Queens> ::On(square, (Board[White] | Board[Black])) & (Board[Queens]  & Board[Color])) return true;
    // if (Attack<King>::On(square) & (Board[King] & Board[Color])) return true;
    // return false;
}

template <EnumColor Color>
std::uint64_t GetAttackedSquares() {
    std::uint64_t attacked = 0ULL;
    for (int square = EnumSquare::a1; square <= EnumSquare::h8; square++)
        if (IsSquareAttacked<Color>(square)) attacked |= Utils::SetSquare(attacked, square);
    return attacked;
}

int main() {
    std::cout << Board << std::endl;
    Utils::Print(GetAttackedSquares<White>());
    return 0;

}
