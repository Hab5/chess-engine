#pragma once

#include "ChessEngine.hpp"
#include "MoveGeneration.hpp"
#include "Evalutation.hpp"
#include "MoveOrdering.hpp"

#include <algorithm>

#define CHECKMATE 32000
#define STALEMATE 00000
#define INFINITY  50000

class PrincipalVariation final { friend class Search;
public:
    static inline auto UpdateLength(std::uint8_t ply) noexcept { length[ply] = ply; }

    static inline auto UpdateTable(std::uint8_t ply, Move move) noexcept {
        table[ply][ply] = move;
        std::copy_n(&table[ply+1][ply+1], length[ply+1], &table[ply][ply+1]);
        length[ply] = length[ply+1];
    }

    [[nodiscard]] static inline auto ToString() noexcept {
        std::stringstream pv;
        for (auto index = 0; index < length[0]; ++index)
            pv << table[0][index] << " ";
        return pv.str();
    }

    [[nodiscard]] static inline auto GetBestMove() { return table[0][0]; }

private:
    static inline std::array<std::array<Move, 64>, 64> table  { };
    static inline std::array<std::uint8_t, 64>         length { };

     PrincipalVariation()=delete;
    ~PrincipalVariation()=delete;
};

class Search final {
public:
    static inline std::uint64_t nodes         = 0;
    static inline Move          best_move;
    static inline std::int16_t  best_score    = 0;
    static inline std::uint8_t  ply           = 0;

    static inline auto Init() noexcept {
        Search::nodes         = 0;
        Search::best_score    = 0;
        Search::ply           = 0;
    }

    [[nodiscard]] static auto AlphaBetaNegamax(GameState& Board, std::uint8_t depth) noexcept {
        return Search::Init(), Board.to_play == White ?
            Negamax<White>(Board, -INFINITY, INFINITY, depth) :
            Negamax<Black>(Board, -INFINITY, INFINITY, depth) ;
    }

    template <EnumColor Color> [[nodiscard]]
    static inline int Negamax(GameState& Board, int alpha, int beta, int depth) noexcept {
        constexpr auto Other = ~Color;

        PrincipalVariation::UpdateLength(Search::ply++);

        if (depth == 0) return --Search::ply, Evaluation::Run<Color>(Board);

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);
        MoveOrdering::SortAll(Board, move_list, nmoves);

        GameState Old = Board; auto legal_moves = 0;
        for (auto move_index = 0; move_index < nmoves; move_index++) {
            auto move = move_list[move_index];
            if (Move::Make<Color>(Board, move)) { ++Search::nodes, ++legal_moves;
                auto score = -Negamax<Other>(Board, -beta, -alpha, depth-1);
                if (score > alpha) {
                    PrincipalVariation::UpdateTable(Search::ply-1, move);
                    if (score >= beta) return --Search::ply, beta;
                    alpha = score;
                }
            } Board = Old;
        }

        if (!legal_moves) {
            if (GameState::InCheck<Color>(Board, Utils::IndexLS1B(Board[King] & Board[Color])))
                return -CHECKMATE + --Search::ply + 1;
            else return --Search::ply, STALEMATE;
        }

        return --Search::ply, alpha;
    }



















    template <EnumColor Color> [[nodiscard]]
    static inline int Quiescence(GameState& Board, int alpha, int beta) noexcept {
        constexpr auto Other = ~Color;

        int score = Evaluation::Run<Color>(Board);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);

        MoveOrdering::SwapFirst(Board, move_list, nmoves);

        GameState Old = Board;

        for (auto move_index = 0; move_index < nmoves; move_index++) {
            auto current_move = move_list[move_index];
            if (current_move.flags & Capture) {
                if (Move::Make<Color>(Board, current_move)) { ++Search::nodes;
                    auto score = -Quiescence<Other>(Board, -beta, -alpha);
                    if (score > alpha) {
                        if (score >= beta) return beta;
                        alpha = score;
                    }
                }
            } Board = Old;
        }

        return alpha;
    }

private:
     Search()=delete;
    ~Search()=delete;
};
