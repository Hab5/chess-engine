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

// STARTING_POS SORTALL (best e2e4)
// info depth 8 score cp -15 nodes 1473910 time 217 nps 6792211 pv e2e4 g8f6 e4e5 f6d5 c2c4 d5f4 g2g4 b8c6

// KIWIPETE SORTALL (best e2a6)
// info depth 8 score cp -200 nodes 7151222 time 2964 nps 2412692 pv e2a6 b4c3 d2c3 e6d5 e5g4 e7e4 f3e4 f6e4

// WIKI_POS_4 SORTALL (best c4c5)
//info depth 8 score cp -555 nodes 1010004 time 300 nps 3366679 pv c4c5 a3b4 a1b1 g7h6 c5b6 g6e4 f1e1 b4a4

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

    GameState Board(PHILIDORS_MATE);
    std::cout << Board << std::endl;
    UCI::Hook(Board);


    return 0;
}
