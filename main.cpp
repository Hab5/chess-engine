#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <functional>
#include <numeric>

enum class LERF {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

enum EnumPiece: std::uint8_t {
    PAWNS,
    KNIGHTS,
    BISHOPS,
    ROOKS,
    QUEENS,
    KING,
};

class ChessBoard final {
public:
    template <typename T>
    std::uint64_t operator[](T flags) const {
        return pieces[flags];
    }

    template<typename T>
    std::uint64_t operator+(T flags) const {

    }

    static void Print(std::uint64_t bitboard) {
        auto str = std::bitset<64>(bitboard).to_string();
        for (int i = 0; i < str.size();)
            std::cout << str[i++] << (!(i % 8) ? '\n':' ');
        std::cout << std::endl;
    }

private:
    std::array<std::uint64_t, 6> pieces = { 1, 2, 3, 4, 5, 6};
};

int main() {
    ChessBoard chessboard;
    ChessBoard::Print(chessboard[PAWNS]);
    for (int i = 0; i < 96/2; i++) {
        for (int j = 0; j < 96; j++)
            std::cout << '*';
        std::cout << '\n';
    }
}
