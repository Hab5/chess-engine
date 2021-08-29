#pragma once

#include "ChessEngine.hpp"
#include "MoveGeneration.hpp"
#include "Evalutation.hpp"
#include "MoveOrdering.hpp"

#include <algorithm>
#include <cstring>

#define CHECKMATE 32000
#define STALEMATE 00000
#define INFINITY  50000


class Search final {
public:
    static inline std::uint64_t nodes;
    static inline std::uint8_t  ply;

    static inline auto Init() noexcept {
        Search::nodes   = 0;
        Search::ply     = 0;
    }

    [[nodiscard]] static auto AlphaBetaNegamax(GameState& Board, std::uint8_t depth) noexcept {
        return Board.to_play == White ?
            Negamax<White>(Board, -INFINITY, INFINITY, depth) :
            Negamax<Black>(Board, -INFINITY, INFINITY, depth) ;
    }

    template <EnumColor Color> [[nodiscard]]
    static inline int Negamax(GameState& Board, int alpha, int beta, int depth) noexcept {
        constexpr auto Other = ~Color; ++Search::nodes; ++Search::ply;

        PrincipalVariation::UpdateLength(Search::ply-1);

        if (depth == 0) return --Search::ply, Evaluation::Run<Color>(Board);

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);
        MoveOrdering::SwapFirst(Board, move_list, nmoves, Search::ply);

        GameState Old = Board; auto legal_moves = 0;
        for (auto move_index = 0; move_index < nmoves; move_index++) {
            auto move = move_list[move_index];
            if (Move::Make<Color>(Board, move)) { ++legal_moves;
                auto score = -Negamax<Other>(Board, -beta, -alpha, depth-1);
                if (score > alpha) {
                    if (score >= beta) return --Search::ply, beta;
                    PrincipalVariation::UpdateTable(Search::ply-1, move);
                    alpha = score;
                }
            } Board = Old;
        }

        if (!legal_moves) {
            if (GameState::InCheck<Color>(Board, Utils::IndexLS1B(Board[King] & Board[Color])))
                return -CHECKMATE + Search::ply--;
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

        // MoveOrdering::SwapFirst(Board, move_list, nmoves);

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
