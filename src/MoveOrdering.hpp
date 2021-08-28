#pragma once

#include "GameState.hpp"
#include "Move.hpp"

#include <algorithm>

class MoveOrdering final {
public:

    static inline auto SortAll(const GameState& Board, MoveList& move_list, int nmoves) noexcept {
        std::sort(&move_list[0], &move_list[nmoves], [&](Move& a, Move& b) {
            return MoveOrdering::MVV_LVA(Board, a) > MoveOrdering::MVV_LVA(Board, b);
        });
    }

    static inline auto PartialSort(const GameState& Board, MoveList& move_list, int nmoves) noexcept {
        std::partial_sort(&move_list[0], &move_list[5], &move_list[nmoves], [&](Move& a, Move& b) {
            return MoveOrdering::MVV_LVA(Board, a) > MoveOrdering::MVV_LVA(Board, b);
        });
    }

    static inline auto SwapFirst(const GameState& Board, MoveList& move_list, int nmoves) {
        auto res = std::max_element(&move_list[0], &move_list[nmoves], [&](Move& a, Move& b) {
            return MoveOrdering::MVV_LVA(Board, a) < MoveOrdering::MVV_LVA(Board, b);});
        std::swap(move_list[0], move_list[std::distance(&move_list[0], res)]);
    }

    static inline int MVV_LVA(const GameState& Board, Move& move) noexcept {
        if (move.flags & Capture) {
            auto victim = Pawns;
            for (int piece = Pawns; piece != King; ++piece)
                if (move.target & Board[piece]) victim = static_cast<EnumPiece>(piece);
            return mvv_lva_table[move.piece-2][victim-2];
        } else return 0;
    }

private:
    static constexpr std::array<std::array<std::uint8_t, 6>, 6> mvv_lva_table {
    //    P   N   B   R   Q   K
        {{15, 25, 35, 45, 55, 65},  // P
         {14, 24, 34, 44, 54, 64},  // N
         {13, 23, 33, 43, 53, 63},  // B
         {12, 22, 32, 45, 52, 62},  // R
         {11, 21, 31, 41, 51, 61},  // Q
         {10, 20, 30, 40, 50, 60},} // K
    };
};
