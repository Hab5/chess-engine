#pragma once

#include "ChessEngine.hpp"
#include "MoveGeneration.hpp"

template <EnumColor Color>
constexpr inline std::array<std::array<int, 64>, 6> PieceSquareScore{};

template <>
constexpr inline std::array<std::array<int, 64>, 6> PieceSquareScore<White> {
    {{
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0, -10, -10,   0,   0,   0,
        0,   0,   0,   5,   5,   0,   0,   0,
        5,   5,  10,  20,  20,   5,   5,   5,
        10,  10,  10,  20,  20,  10,  10,  10,
        20,  20,  20,  30,  30,  30,  20,  20,
        30,  30,  30,  40,  40,  30,  30,  30,
        90,  90,  90,  90,  90,  90,  90,  90,
    },

    { // KNIGHTS
        -5, -10,   0,   0,   0,   0, -10,  -5
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   5,  20,  10,  10,  20,   5,  -5,
        -5,  10,  20,  30,  30,  20,  10,  -5,
        -5,  10,  20,  30,  30,  20,  10,  -5,
        -5,   5,  20,  20,  20,  20,   5,  -5,
        -5,   0,   0,  10,  10,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,

    },

    { // BISHOPS
        0,   0, -10,   0,   0, -10,   0,   0,
        0,  30,   0,   0,   0,   0,  30,   0,
        0,  10,   0,   0,   0,   0,  10,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,   0,  10,  10,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
    },

    { // ROOKS
        0,   0,   0,  20,  20,   0,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        50,  50,  50,  50,  50,  50,  50,  50,
        50,  50,  50,  50,  50,  50,  50,  50,
    },

    { }, // QUEENS
         //
    { // KING
        0,   0,   5,   0, -15,   0,  10,   0,
        0,   5,   5,  -5,  -5,   0,   5,   0,
        0,   0,   5,  10,  10,   5,   0,   0,
        0,   5,  10,  20,  20,  10,   5,   0,
        0,   5,  10,  20,  20,  10,   5,   0,
        0,   5,   5,  10,  10,   5,   5,   0,
        0,   0,   5,   5,   5,   5,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
    }}
};

template <>
constexpr inline std::array<std::array<int, 64>, 6> PieceSquareScore<Black> {
    {{
        90,  90,  90,  90,  90,  90,  90,  90,
        30,  30,  30,  40,  40,  30,  30,  30,
        20,  20,  20,  30,  30,  30,  20,  20,
        10,  10,  10,  20,  20,  10,  10,  10,
        5,   5,  10,  20,  20,   5,   5,   5,
        0,   0,   0,   5,   5,   0,   0,   0,
        0,   0,   0, -10, -10,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0
    },

    { // KNIGHTS
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,  10,  10,   0,   0,  -5,
        -5,   5,  20,  20,  20,  20,   5,  -5,
        -5,  10,  20,  30,  30,  20,  10,  -5,
        -5,  10,  20,  30,  30,  20,  10,  -5,
        -5,   5,  20,  10,  10,  20,   5,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5, -10,   0,   0,   0,   0, -10,  -5

    },

    { // BISHOPS
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,  10,  10,   0,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,  10,   0,   0,   0,   0,  10,   0,
        0,  30,   0,   0,   0,   0,  30,   0,
        0,   0, -10,   0,   0, -10,   0,   0
    },

    { // ROOKS
        50,  50,  50,  50,  50,  50,  50,  50,
        50,  50,  50,  50,  50,  50,  50,  50,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,  10,  20,  20,  10,   0,   0,
        0,   0,   0,  20,  20,   0,   0,   0
    },

    { },// QUEENS

    { // KING
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   5,   5,   5,   5,   0,   0,
        0,   5,   5,  10,  10,   5,   5,   0,
        0,   5,  10,  20,  20,  10,   5,   0,
        0,   5,  10,  20,  20,  10,   5,   0,
        0,   0,   5,  10,  10,   5,   0,   0,
        0,   5,   5,  -5,  -5,   0,   5,   0,
        0,   0,   5,   0, -15,   0,  10,   0
    }}
};


class Evaluation final {
public:
    template <EnumColor Color>
    [[nodiscard]] static inline auto Run(GameState& Board) noexcept {
        constexpr auto Relative = (Color == White ? 1 : -1);
        constexpr int PieceValue[6] {100, 300, 300, 500, 900, 10000};
        auto material_score = 0; int index = 0;
        auto WhitePieces = Board[White], BlackPieces = Board[Black];
        for (auto set = &Board[Pawns]; set <= &Board[King]; ++set, ++index) {
            auto white_set = *set & WhitePieces, black_set = *set & BlackPieces;
            while (white_set) {
                auto square = Utils::PopLS1B(white_set);
                material_score += PieceValue[index] + PieceSquareScore<White>[index][square];
            } while (black_set) {
                auto square = Utils::PopLS1B(black_set);
                material_score -= PieceValue[index] + PieceSquareScore<Black>[index][square];
            }
        } return Relative * material_score;
    };
};
