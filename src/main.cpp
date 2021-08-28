#include "Move.hpp"
#include "UCI.hpp"
#include "Perft.hpp"
#include "Search.hpp"
#include "MoveOrdering.hpp"

#include <algorithm>
#include <random>



int main(int argc, char* argv[]) { (void)argc; (void)argv;
    // GameState Board(STARTING_POSITION);
    GameState Board("4r2k/2pRP1pp/2p5/p4pN1/2Q3n1/q5P1/P3PP1P/6K1 w - - 0 1"); // philidor's mate
    // GameState Board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"); // kiwpete
    // GameState Board("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"); // pos 4
    // GameState Board("rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"); // killer


    // auto [move, score] = Search::AlphaBetaNegamax(Board, 7);
    // std::cout << Board << std::endl;
    if (argc != 1) Perft::Run(Board, std::atoi(argv[1]));
    else           UCI::Hook(Board);

    return 0;
}
