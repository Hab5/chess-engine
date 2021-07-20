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

enum EnumSquare { // LERF
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
    White = 0x00,
    Black = 0x01
};

enum EnumPiece: std::uint8_t {
    Pawns   = 0x02,
    Knights = 0x03,
    Bishops = 0x04,
    Rooks   = 0x05,
    Queens  = 0x06,
    King    = 0x07
};

enum EnumFlip: std::uint8_t {
    Vertical,
    Horizontal,
    Diagonal
};

enum EnumFile: std::uint64_t {
    FileA = 0x0101010101010101,
    FileB = 0x0202020202020202,
    FileC = 0x0303030303030303,
    FileD = 0x0404040404040404,
    FileE = 0x0505050505050505,
    FileF = 0x0606060606060606,
    FileG = 0x0707070707070707,
    FileH = 0x0808080808080808
};

enum EnumRank: std::uint64_t {
    Rank1 = 0x00000000000000ff,
    Rank2 = 0x000000000000ff00,
    Rank3 = 0x0000000000ff0000,
    Rank4 = 0x00000000ff000000,
    Rank5 = 0x000000ff00000000,
    Rank6 = 0x0000ff0000000000,
    Rank7 = 0x00ff000000000000,
    Rank8 = 0xff00000000000000
};

class BBTools final {
public:
    static void Print(std::uint64_t bitboard) {
        auto str = std::bitset<64>(bitboard).to_string();
        for (int i = 0; i < str.size();)
            std::cout << str[i++] << (!(i % 8) ? '\n':' ');
        std::cout << std::endl;
    }

    template <EnumFlip flip>
    [[nodiscard]] static auto Flip(std::uint64_t bitboard) {
        if (flip == Vertical)
            return __builtin_bswap64(bitboard);
        else if (flip == Horizontal) {
            const std::uint64_t k1 = 0x5555555555555555;
            const std::uint64_t k2 = 0x3333333333333333;
            const std::uint64_t k4 = 0x0f0f0f0f0f0f0f0f;
            bitboard = ((bitboard >> 1) & k1) +  2*(bitboard & k1);
            bitboard = ((bitboard >> 2) & k2) +  4*(bitboard & k2);
            bitboard = ((bitboard >> 4) & k4) + 16*(bitboard & k4);
            return bitboard;
        }
    }
};

struct Pieces final {
public:
    template <typename T>
    [[nodiscard]] inline auto operator[](T query) const {
        return boards[query];
    }

    template<EnumColor c, EnumPiece p>
    auto constexpr SetBit(EnumSquare sq) {
        boards[p] |= (1ULL << sq);
        boards[c] |= (1ULL << sq);
    }

    template <EnumColor c, EnumPiece p>
    [[nodiscard]] auto constexpr GetMoves() {
        std::uint64_t piece_set =   boards[p] & boards[c];
        std::uint64_t empty     = ~(boards[c] | boards[c]);
        std::uint64_t enemy     =   boards[!c];

        switch (p) {
        case Pawns:
        case Knights:
        case Bishops:
        case Rooks:
        case Queens:
        case King: ;
        }

        return enemy;
    }

private:
    std::array<std::uint64_t, 8> boards {
        (0xffffULL << 0 ) | 0x0000ULL, // White
        (0xffffULL << 48) | 0x0000ULL, // Black
        (0xff00ULL << 40) | 0xff00ULL, // Pawns
        (0x0042ULL << 56) | 0x0042ULL, // Knights
        (0x0024ULL << 56) | 0x0024ULL, // Bishops
        (0x0081ULL << 56) | 0x0081ULL, // Rooks
        (0x0008ULL << 56) | 0x0008ULL, // Queens
        (0x0010ULL << 56) | 0x0010ULL  // King
    };
};

int main() {
    Pieces Pieces;
    BBTools::Print(Pieces[Pawns]);
    Pieces.SetBit<White, Pawns>(a3);
    BBTools::Print(Pieces[White] & FileA);
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
