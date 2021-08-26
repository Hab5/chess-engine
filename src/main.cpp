#include "Move.hpp"
#include "UCI.hpp"
#include "Perft.hpp"
#include "Search.hpp"

#include <algorithm>
#include <random>

int main(int argc, char* argv[]) { (void)argc; (void)argv;
    GameState Board(STARTING_POSITION);

    if (argc != 1) Perft::Run(Board, std::atoi(argv[1]));
    else           UCI::Hook(Board);

    return 0;
}
