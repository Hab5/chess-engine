#include "ChessEngine.hpp"
#include "ChessBoard.hpp"
#include "Attack.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#include <iostream>
#include <vector>
#include <array>

int main() {
    ChessBoard Board("rnb1kbnr/pp3ppp/2p5/3Pp1q1/3P4/5N2/PPP2PPP/RNBQKB1R w KQkq - 1 5");
    std::cout << Board << std::endl;
    return 0;
}
