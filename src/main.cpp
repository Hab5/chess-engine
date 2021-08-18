#include "ChessEngine.hpp"
#include "GameState.hpp"
#include "GetAttack.hpp"
#include "Perft.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#define WPOS "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define BPOS "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1"

#define POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"
int main(int argc, char* argv[]) { (void)argc;

    GameState Board(STARTING_POSITION);
    Perft::Run(Board, std::atoi(argv[1]));

    return 0;
}
