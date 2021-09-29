#pragma once

#include "GameState.hpp"
#include "Move.hpp"

#include <algorithm>
#include <cstring>

class PrincipalVariation final { friend class Search;
public:

    [[nodiscard]] static inline auto  GetBestMove()              { return table[0][0];   }
    [[nodiscard]] static inline auto& GetMove(std::uint8_t ply)  { return table[0][ply]; }

    [[nodiscard]] static inline auto ToString() noexcept {
        std::stringstream pv;
        for (auto index = 0; index < length[0]; ++index)
            pv << table[0][index] << " ";
        return pv.str();
    }


private:
    static inline std::array<std::array<Move, 64>, 64> table  { };
    static inline std::array<std::uint8_t, 64>         length { };

    static inline auto UpdateLength(std::uint8_t ply) noexcept { length[ply] = ply; }

    static inline auto UpdateTable(std::uint8_t ply, Move move) noexcept {
        table[ply][ply] = move;
        std::copy_n(&table[ply+1][ply+1], length[ply+1], &table[ply][ply+1]);
        length[ply] = length[ply+1];
    }

     PrincipalVariation()=delete;
    ~PrincipalVariation()=delete;
};

class MoveOrdering final {
public:

    static inline int ScoreMove
    (const GameState& Board, Move& move, std::uint8_t ply) noexcept {
        if (*reinterpret_cast<int*>(&move) ==
            *reinterpret_cast<int*>(&PrincipalVariation::GetMove(ply-1))) {
            return 100;
        } else if (move.flags & Capture) {
            auto victim = Pawns;
            for (int piece = Pawns; piece != King; ++piece) {
                if (move.target & Board[piece]) {
                    victim = static_cast<EnumPiece>(piece);
                    break;
                }
            } return mvv_lva_table[move.piece-2][victim-2];
        } else return 0;
    }

    static inline auto SortAll
    (const GameState& Board, MoveList& move_list, int nmoves, std::uint8_t ply) noexcept {
        std::sort(&move_list[0], &move_list[nmoves], [&](Move& a, Move& b) {
            return MoveOrdering::ScoreMove(Board, a, ply) > MoveOrdering::ScoreMove(Board, b, ply);
        });
    }

    static inline auto SwapFirst
    (const GameState& Board, MoveList& move_list, int nmoves, std::uint8_t ply) {
        auto res = std::max_element(&move_list[0], &move_list[nmoves], [&](Move& a, Move& b) {
            return MoveOrdering::ScoreMove(Board, a, ply) < MoveOrdering::ScoreMove(Board, b, ply);});
        std::swap(move_list[0], move_list[std::distance(&move_list[0], res)]);
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
