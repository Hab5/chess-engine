#include "FEN.hpp"
#include "GetAttack.hpp"
#include "Move.hpp"
#include "MoveGeneration.hpp"
#include "UCI.hpp"
#include "Perft.hpp"
#include "Search.hpp"
#include "MoveOrdering.hpp"
#include "TranspositionTable.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#define STARTING_BLACK "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"
#define PHILIDORS_MATE "4r2k/2pRP1pp/2p5/p4pN1/2Q3n1/q5P1/P3PP1P/6K1 w - - 0 1"
#define KIWIPETE       "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define WIKI_POS_4     "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define WIKI_POS_5     "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
#define KILLER         "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"

int main(int argc, char* argv[]) { (void)argc; (void)argv;
    if (argc != 1) {
        GameState Board(STARTING_POSITION);
        if (std::strcmp(argv[1], "pgo") == 0)
            Perft::Run(Board, 6), (void)Search::AlphaBetaNegamax(Board, 4);
        else Perft::Run(Board, std::atoi(argv[1]));
        return 0;
    }

    GameState Board(KILLER);
    std::cout << Board << std::endl;
    (void)Search::AlphaBetaNegamax(Board, 6);
    /* UCI::Hook(Board); */

    return 0;
}
