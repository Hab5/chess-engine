#pragma once

#include "GameState.hpp"
#include "MoveGeneration.hpp"

#include <chrono>
#include <iomanip>

class Perft final {
public:
    static std::uint64_t Run(GameState& Board, int depth) noexcept {
        std::uint64_t nodes = 0;

        auto started  = std::chrono::steady_clock::now();

        if (depth % 2 == 0) nodes = (Board.to_play ==
            White ? EvenPerft<White>(Board, depth) :
                    EvenPerft<Black>(Board, depth));

        if (depth % 2 != 0) nodes = (Board.to_play ==
            White ? OddPerft<White>(Board, depth) :
                    OddPerft<Black>(Board, depth));

        auto finished = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast
                    <std::chrono::milliseconds>
                    (finished-started).count();

        auto sec = ms / 1000.00000f;
        std::cout.imbue(std::locale(""));
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "[depth=" << depth << "][" << nodes << "]["
            << "" << std::setprecision(2) << nodes/sec/1000000 << "Mnps]\n";

        return nodes;
    }

private:

    template <EnumColor Color> __attribute__((always_inline))
    static inline std::uint64_t EvenPerft(GameState& Board, int depth) noexcept {
        constexpr auto Other = ~Color;
        if (depth == 0) return 1ULL;

        auto [move_list, nmoves] = MoveGen::Run<Color>(Board);

        GameState Old = Board;
        std::uint64_t nodes = 0;
        for (auto move = 0; move < nmoves; move++) {
            if (Move::Make<Color>(Board, move_list[move]))
                nodes += OddPerft<Other>(Board, depth-1);
            Board = Old;
        } return nodes;
    }

    template <EnumColor Color> __attribute__((noinline))
    static inline std::uint64_t OddPerft(GameState& Board, int depth) noexcept {
        constexpr auto Other = ~Color;
        if (depth == 0) return 1ULL;

        auto [move_list, nmoves] = MoveGen::Run<Color>(Board);

        GameState Old = Board;
        std::uint64_t nodes = 0;
        for (auto move = 0; move < nmoves; move++) {
            if (Move::Make<Color>(Board, move_list[move]))
                nodes += EvenPerft<Other>(Board, depth-1);
            Board = Old;
        } return nodes;
    }

     Perft()=delete;
    ~Perft()=delete;
};
