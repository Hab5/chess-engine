#pragma once

#include "ChessEngine.hpp"
#include "Utils.hpp"

#include <exception>
#include <string>
#include <array>
#include <sstream>
#include <algorithm>

#define STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

class GameState;
class FEN final {
public:
    static void Load(const std::string& fen, GameState& position);

private:
    static void LoadPieces(std::string ranks, GameState& board);
    static void LoadActiveColor(std::string active_color, GameState& board);
    static void LoadCastlingRights(std::string castling_rights, GameState& board);
    static void LoadEnPassant(std::string en_passant, GameState& board);
    static void LoadHalfMoves(std::string half_moves, GameState& board, bool stream_ok);
    static void LoadFullMoves(std::string full_moves, GameState& board, bool stream_ok);

     FEN() = delete;
    ~FEN() = delete;
};
