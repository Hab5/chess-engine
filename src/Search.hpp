#pragma once

#include "ChessEngine.hpp"
#include "MoveGeneration.hpp"
#include "Evalutation.hpp"


#define CHECKMATE 32000
#define STALEMATE 00000
#define INFINITY  50000

class Search final {
public:
    static inline std::uint8_t  root_depth    = 0;
    static inline std::uint64_t nodes         = 0;
    static inline std::int16_t  best_score    = 0;
    static inline Move          best_move;

    static inline auto Init(std::uint8_t depth) noexcept {
        Search::root_depth    = depth;
        Search::nodes         = 0;
        Search::best_score    = 0;
    }

    [[nodiscard]]
    static auto AlphaBetaNegamax(GameState& Board, std::uint8_t depth) noexcept {
        return Search::Init(depth), Board.to_play == White ?
            NegamaxRoot<White>(Board, -INFINITY, INFINITY, depth) :
            NegamaxRoot<Black>(Board, -INFINITY, INFINITY, depth) ;
    }

    template <EnumColor Color> [[nodiscard]]
    static inline auto NegamaxRoot(GameState& Board, int alpha, int beta, int depth)
        -> std::tuple<Move, std::int16_t> {
        constexpr auto Other = ~Color;

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);

        GameState Old = Board;

        for (auto move_index = 0; move_index < nmoves; move_index++) {
            auto current_move = move_list[move_index];
            if (Move::Make<Color>(Board, current_move)) { ++Search::nodes;
                auto score = -Negamax<Other>(Board, -beta, -alpha, depth-1);
                if (score > alpha) {
                    alpha = score;
                    Search::best_move  = current_move;
                    Search::best_score = score;
                }
            } Board = Old;
        }
        return { Search::best_move, Search::best_score };
    }


    template <EnumColor Color> [[nodiscard]]
    static inline int Negamax(GameState& Board, int alpha, int beta, int depth) {
        constexpr auto Other = ~Color;
        if (depth == 0) return Evaluation::Run<Color>(Board); //Quiescence<Color>(Board, alpha, beta);

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);

        GameState Old = Board;

        auto legal_moves = 0;
        for (auto move_index = 0; move_index < nmoves; move_index++) {
            auto current_move = move_list[move_index];
            if (Move::Make<Color>(Board, current_move)) { ++Search::nodes, ++legal_moves;
                auto score = -Negamax<Other>(Board, -beta, -alpha, depth-1);
                if (score > alpha) {
                    if (score >= beta) return beta;
                    alpha = score;
                }
            } Board = Old;
        }

        if (!legal_moves) {
            if (GameState::InCheck<Color>(Board, Utils::IndexLS1B(Board[King] & Board[Color])))
                return -CHECKMATE - (Search::root_depth - depth);
            else return STALEMATE;
        }

        return alpha;
    }

    template <EnumColor Color> [[nodiscard]]
    static inline int Quiescence(GameState& Board, int alpha, int beta) {
        constexpr auto Other = ~Color;

        int score = Evaluation::Run<Color>(Board);
        if (score >= beta) return beta;
        if (alpha < score) alpha = score;

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);
        GameState Old = Board;

        auto legal_moves = 0;
        for (auto move_index = 0; move_index < nmoves; move_index++) { legal_moves++;
            auto current_move = move_list[move_index];
            if (current_move.flags & Capture) {
                if (Move::Make<Color>(Board, current_move)) {
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
};
