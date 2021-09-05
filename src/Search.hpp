#pragma once

#include "ChessEngine.hpp"
#include "MoveGeneration.hpp"
#include "Evalutation.hpp"
#include "MoveOrdering.hpp"

#include <algorithm>
#include <cstring>


#define CHECKMATE  32000
#define STALEMATE  00000
#define INF        50000

class Search final {
public:
    static inline std::uint64_t nodes;
    static inline std::uint8_t  ply;

    static inline auto Init() noexcept {
        std::memset(&PrincipalVariation::table,  0, sizeof(PrincipalVariation::table));
        std::memset(&PrincipalVariation::length, 0, sizeof(PrincipalVariation::length));
        Search::nodes = 0, Search::ply   = 0;
    }

    [[nodiscard]] static auto AlphaBetaNegamax
    (GameState& Board, int depth) noexcept {
        return Board.to_play == White ?
            Negamax<White>(Board, -INF, INF, depth) :
            Negamax<Black>(Board, -INF, INF, depth) ;
    }

    template <EnumColor Color> [[nodiscard]]
    static inline int Negamax(GameState& Board, int alpha, int beta, int depth) noexcept {
        constexpr auto Other = ~Color; ++Search::nodes; ++Search::ply;
        int PrincipalVariationSearch = 0;

        PrincipalVariation::UpdateLength(Search::ply-1);

        if (depth == 0) return --Search::ply, Evaluation::Run<Color>(Board);

        if (NullMovePruning<Other>(Board, beta, depth) >= beta)
            return --Search::ply, beta;

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);

        // MoveOrdering::SwapFirst(Board, move_list, nmoves, Search::ply);
        MoveOrdering::SortAll(Board, move_list, nmoves, Search::ply);

        GameState Old = Board; auto legal_moves = 0; auto score = 0;
        for (auto move_index = 0; move_index < nmoves; move_index++) {
            auto move = move_list[move_index];
            if (Move::Make<Color>(Board, move)) { ++legal_moves;

                if (PrincipalVariationSearch) {
                    score = -Negamax<Other>(Board, -alpha-1, -alpha, depth-1);
                    if (score > alpha && score < beta)
                        score = -Negamax<Other>(Board, -beta, -alpha, depth-1);
                } else  score = -Negamax<Other>(Board, -beta, -alpha, depth-1);

                if (score > alpha) { PrincipalVariationSearch = 1;
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

    template <EnumColor Other>
    static inline int NullMovePruning(GameState& Board, int beta, int depth) noexcept {
        constexpr auto R = 3;
        if (depth >= R+1 && ply-1) {
            GameState Old = Board;
            Board.to_play = Other;
            Board.en_passant = EnumSquare(0);
            auto score = -Negamax<Other>(Board, -beta, -beta + 1, depth-1 - R);
            Board = Old; return score;
        } else return -INF;
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
