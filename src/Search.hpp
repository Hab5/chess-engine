#pragma once

#include "Move.hpp"
#include "TranspositionTable.hpp"
#include "MoveGeneration.hpp"
#include "MoveOrdering.hpp"
#include "ChessEngine.hpp"
#include "Evalutation.hpp"

#include <algorithm>
#include <cstring>
#include <atomic>

#define CHECKMATE  32000
#define STALEMATE  00000
#define INF        50000

static TranspositionTable HashTable;

class Search final { friend class UCI;
public:
    static inline std::uint64_t nodes;
    static inline std::uint8_t  ply;

    static inline auto Init() noexcept {
        // HashTable.Clear();
        std::memset(&PrincipalVariation::table,  0, sizeof(PrincipalVariation::table));
        std::memset(&PrincipalVariation::length, 0, sizeof(PrincipalVariation::length));
        Search::nodes = 0, Search::ply = 0;
    }

    [[nodiscard]] static auto AlphaBetaNegamax
    (GameState& Board, int depth) noexcept {
        return Board.to_play == White ?
            Negamax<White>(Board, -INF, INF, depth) :
            Negamax<Black>(Board, -INF, INF, depth) ;
    }

private:
    static inline std::atomic<bool> stop = false;

    template <EnumColor Color> [[nodiscard]]
    static inline int Negamax(GameState& Board, int alpha, int beta, int depth) noexcept {
        constexpr auto Other = ~Color; ++Search::nodes; auto score = 0;

        if (stop) return beta;

        bool PrincipalVariationSearch = false;

        PrincipalVariation::UpdateLength(Search::ply);

        if (depth == 0) return Evaluation::Run<Color>(Board);
        // if (depth == 0) return Search::Quiescence<Color>(Board, alpha, beta);

        TTFlag HashFlag = HashAlpha;
        if (Search::ply && (score = HashTable.Probe(Board, alpha, beta, depth) != 0xDEAD))
            return beta; // THE FUCK IS GOING ON HERE?!

        if (NullMovePruning<Other>(Board, beta, depth) >= beta)
            return beta;

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);


        // DO NOT TOUCH THE +1 OR -1 IN FUNCTION, CURSED COMPILER OPTIMIZATIONS
        // MoveOrdering::SwapFirst(Board, move_list, nmoves, Search::ply+1);
        MoveOrdering::SortAll(Board, move_list, nmoves, Search::ply+1);


        GameState Old = Board; Move best_move; auto legal_moves = 0;
        for (auto move_index = 0; move_index < nmoves; move_index++) {
            auto move = move_list[move_index];
            if (Move::Make<Color>(Board, move)) { ++legal_moves;

                ++Search::ply;
                if (PrincipalVariationSearch) {
                    score = -Negamax<Other>(Board, -alpha-1, -alpha, depth-1);
                    if (score > alpha && score < beta)
                        score = -Negamax<Other>(Board, -beta, -alpha, depth-1);
                } else  score = -Negamax<Other>(Board, -beta, -alpha, depth-1);
                --Search::ply;

                if (score > alpha) { PrincipalVariationSearch = true;
                    if (score >= beta) {
                        HashTable.Record(Old, HashBeta, score, move, depth);
                        return beta;
                    }
                    alpha = score, best_move = move;
                    PrincipalVariation::UpdateTable(Search::ply, best_move);
                    HashFlag = HashExact;
                }
            } Board = Old;
        }

        if (!legal_moves) {
            if (GameState::InCheck<Color>(Board, Utils::IndexLS1B(Board[King] & Board[Color])))
                return -CHECKMATE + Search::ply+1;
            else return STALEMATE;
        }

        HashTable.Record(Board, HashFlag, alpha, best_move, depth);
        return alpha;
    }

    template <EnumColor Other> [[nodiscard]]
    static inline int NullMovePruning(GameState& Board, int beta, int depth) noexcept {
        constexpr auto R = 3;
        if (depth >= R+1 && ply) {
            GameState Old = Board;
            if (Board.en_passant)
                Board.hash ^= ZobristHashing::Keys.EnPassant[Board.en_passant];
            Board.hash ^= ZobristHashing::Keys.Side;
            Board.to_play = Other;
            Board.en_passant = EnumSquare(0);
            ++Search::ply;
            auto score = -Negamax<Other>(Board, -beta, -beta + 1, depth-1 - R);
            --Search::ply;
            Board = Old; return score;
        } else return -INF;
    }

    template <EnumColor Color> [[nodiscard]]
    static inline int Quiescence(GameState& Board, int alpha, int beta) noexcept {
        constexpr auto Other = ~Color; ++Search::nodes;

        int score = Evaluation::Run<Color>(Board);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);

        MoveOrdering::SortAll(Board, move_list, nmoves, Search::ply+1);

        GameState Old = Board;

        for (auto move_index = 0; move_index < nmoves; move_index++) {
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

     Search()=delete;
    ~Search()=delete;
};
