#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <functional>
#include <numeric>
#include <stdlib.h>
#include <type_traits>

enum EnumSquare {
    a1,b1,c1,d1,e1,f1,g1,h1,
    a2,b2,c2,d2,e2,f2,g2,h2,
    a3,b3,c3,d3,e3,f3,g3,h3,
    a4,b4,c4,d4,e4,f4,g4,h4,
    a5,b5,c5,d5,e5,f5,g5,h5,
    a6,b6,c6,d6,e6,f6,g6,h6,
    a7,b7,c7,d7,e7,f7,g7,h7,
    a8,b8,c8,d8,e8,f8,g8,h8
};

enum EnumColor: std::uint8_t {
    WHITE = 0x00,
    BLACK = 0x01
};

enum EnumPiece: std::uint8_t {
    PAWNS   = 0x02,
    KNIGHTS = 0x03,
    BISHOPS = 0x04,
    ROOKS   = 0x05,
    QUEENS  = 0x06,
    KING    = 0x07
};

enum EnumFlip: std::uint8_t {
    VERTICAL,
    HORIZONTAL,
    DIAGONAL
};

enum DefaultBoardPosition {};

// indebitboard = 8*rank+file
// flip_vertical = __builtin_bswap64(2);

class ChessBoard final {
public:

    ChessBoard() {
        Pieces[WHITE  ] |= (0xffffULL << 48ULL) | 0x0000ULL;
        Pieces[BLACK  ] |= (0xffffULL << 00ULL) | 0x0000ULL;
        Pieces[PAWNS  ] |= (0xff00ULL << 40ULL) | 0xff00ULL;
        Pieces[KNIGHTS] |= (0x0042ULL << 56ULL) | 0x0042ULL;
        Pieces[BISHOPS] |= (0x0024ULL << 56ULL) | 0x0024ULL;
        Pieces[ROOKS  ] |= (0x0081ULL << 56ULL) | 0x0081ULL;
        Pieces[QUEENS ] |= (0x0008ULL << 56ULL) | 0x0008ULL;
        Pieces[KING   ] |= (0x0010ULL << 56ULL) | 0x0010ULL;

    }

    template <typename T>
    [[nodiscard]] inline auto operator[](T query) const {
        return Pieces[query];
    }

    static auto Print(std::uint64_t bitboard) {
        auto str = std::bitset<64>(bitboard).to_string();
        std::reverse(str.begin(), str.end());
        for (int i = 0; i < str.size();)
            std::cout << str[i++] << (!(i % 8) ? '\n':' ');
        std::cout << std::endl;
    }

    template <EnumFlip flip>
    [[nodiscard]] static auto Flip(std::uint64_t bitboard) {
        if (flip == VERTICAL)
            return __builtin_bswap64(bitboard);
        else if (flip == HORIZONTAL) {
            const std::uint64_t k1 = 0x5555555555555555;
            const std::uint64_t k2 = 0x3333333333333333;
            const std::uint64_t k4 = 0x0f0f0f0f0f0f0f0f;
            bitboard = ((bitboard >> 1) & k1) +  2*(bitboard & k1);
            bitboard = ((bitboard >> 2) & k2) +  4*(bitboard & k2);
            bitboard = ((bitboard >> 4) & k4) + 16*(bitboard & k4);
            return bitboard;
        }
    }

private:
    std::array<std::uint64_t, 8> Pieces { };
};



int main() {
    ChessBoard board;
    ChessBoard::Print(board[BLACK]);
}
// Get the white knights/rooks/king on the 1st and 8th ranks and on the C file, then flip the bitboard vertically.
//
// Flip<VERTICAL>((bb[WHITE | KNIGHTS]      |
//                 bb[WHITE | ROOKS]        |
//                 bb[WHITE | KING])        &
//                RANK_1 & RANK_8 & FILE_C);
//
// BITBOARD.QUERY::COLOR<WHITE>::PIECE<KNIGHTS|ROOKS|KING>::ON<RANK_1|RANK_8|FILE_C>::FLIP<VERTICAL>::RESULT;


// board.get<WHITE|BLACK>().pieces(KNIGHTS, ROOKS).on(RANK_1, RANK_8, ~FILE_C).flip(VERTICAL);

// board.get(color<WHITE|BLACK>, pieces)
