#pragma once

#include "ChessEngine.hpp"
#include "Utils.hpp"

#include <exception>
#include <string>
#include <array>
#include <sstream>
#include <algorithm>

#define FEN_DEFAULT "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

class ChessBoard;
class FEN final {
public:
    static void Load(const std::string& fen, ChessBoard& position);

private:
    static void LoadPieces(std::string ranks, ChessBoard& board);
    static void LoadActiveColor(std::string active_color, ChessBoard& board);
    static void LoadCastlingRights(std::string castling_rights, ChessBoard& board);
    static void LoadEnPassant(std::string en_passant, ChessBoard& board);
    static void LoadHalfMoves(std::string half_moves, ChessBoard& board);
    static void LoadFullMoves(std::string full_moves, ChessBoard& board);

     FEN() = delete;
    ~FEN() = delete;
};
