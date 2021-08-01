#include "FEN.hpp"
#include "ChessBoard.hpp"

void FEN::Load(const std::string& fen, ChessBoard& board) {
    std::istringstream stream(fen); std::string token;
    LoadPieces          ((stream >> token, token), board);
    LoadActiveColor     ((stream >> token, token), board);
    LoadCastlingRights  ((stream >> token, token), board);
    LoadEnPassant       ((stream >> token, token), board);
    LoadHalfMoves       ((stream >> token, token), board, stream.good());
    LoadFullMoves       ((stream >> token, token), board, stream.good());
}

void FEN::LoadPieces(std::string ranks, ChessBoard& board) {
    if (std::count(ranks.begin(), ranks.end(), '/') != 7)
        throw std::runtime_error("FEN: Syntax Error");
    const auto pieces_ascii = std::string("PNBRQKpnbrqk");
    std::size_t slash, piece_idx;
    EnumSquare square = h8; ranks += '/';
    while ((slash = ranks.find('/')) != std::string::npos) {
        auto start_square = square;
        auto rank = ranks.substr(0, slash);
        for (auto ch: rank) {
            if (std::isdigit(ch)) {
                if (ch <= '8') square -= ch - '0' - 1;
                else throw std::runtime_error("FEN: Syntax Error");
            } else if ((piece_idx = pieces_ascii.find(ch)) != std::string::npos) {
                auto color = !std::isupper(ch);
                auto piece = (piece_idx) - (color ? 6 : 0) + 2;
                auto bitboard = Utils::MakeSquare(start_square - (square % 8));
                board.pieces[piece] |= bitboard, board.pieces[color] |= bitboard;
            } else throw std::runtime_error("FEN: Syntax Error");
            square--;
        } ranks.erase(0, slash+1);
    }
}

void FEN::LoadActiveColor(std::string active_color, ChessBoard& board) {
    if (active_color.size() != 1 || (active_color[0] != 'w' && active_color[0] != 'b'))
        throw std::runtime_error("FEN: Syntax Error");
    else board.to_play = (active_color[0] == 'w' ? White : Black);
}

void FEN::LoadCastlingRights(std::string castling_rights, ChessBoard& board) {
    if (castling_rights.size() == 1) {
        if ( castling_rights[0] != '-' )
            throw std::runtime_error("FEN: Syntax Error");
        return;
    } else if (castling_rights.size() != 4
        || ((castling_rights[0] != 'K' && castling_rights[0] != '-')
        ||  (castling_rights[1] != 'Q' && castling_rights[1] != '-')
        ||  (castling_rights[2] != 'k' && castling_rights[2] != '-')
        ||  (castling_rights[3] != 'q' && castling_rights[3] != '-')))
        throw std::runtime_error("FEN: Syntax Error");
    else for (int i = 0; i < 4; i++) board.castling_rights[i] = (castling_rights[i] != '-');
}

void FEN::LoadEnPassant(std::string en_passant, ChessBoard& board) {
    if (en_passant.size() > 2)
        throw std::runtime_error("FEN: Syntax Error");
    else if (en_passant.size() == 2) {
        auto it = std::find(SquareIndex.begin(), SquareIndex.end(), en_passant);
        if (it != SquareIndex.end())
            board.en_passant = std::distance(SquareIndex.begin(), it);
        else throw std::runtime_error("FEN: Syntax Error");
    } else if (en_passant[0] == '-') board.en_passant = -1;
    else throw std::runtime_error("FEN: Syntax Error");
}

void FEN::LoadHalfMoves(std::string half_moves, ChessBoard& board, bool stream_ok) {
    if (!stream_ok) return void(board.half_moves = 0);
    if (half_moves.size() > 3) throw std::runtime_error("FEN: Syntax Error");
    for (auto c: half_moves)
        if (!std::isdigit(c))
            throw std::runtime_error("FEN: Syntax Error");
    int value = std::atoi(half_moves.c_str());
    if (value > 100) throw std::runtime_error("FEN: Syntax Error");
    else board.half_moves = value;
}

void FEN::LoadFullMoves(std::string full_moves, ChessBoard& board, bool stream_ok) {
    if (!stream_ok) return void(board.full_moves = 1);
    if (full_moves.size() > 3) throw std::runtime_error("FEN: Syntax Error");
    for (auto c: full_moves)
        if (!std::isdigit(c))
            throw std::runtime_error("FEN: Syntax Error");
    int value = std::atoi(full_moves.c_str());
    if (value > 500) throw std::runtime_error("FEN: Syntax Error");
    else board.full_moves = value;
};
