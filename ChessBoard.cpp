#include "Position.hpp"
#include "FEN.hpp"

ChessBoard::ChessBoard(const std::string& fen) { FEN::Load(fen, *this); }

[[nodiscard]] std::string ChessBoard::Show() const noexcept {
    std::stringstream output;

    const std::array<std::string, 6> unicode { "♟", "♞", "♝", "♜", "♛", "♚" };

    const auto fg_white = "\033[38;5;250m";
    const auto fg_black = "\033[38;5;59m";
    const auto fg_dim   = "\033[38;5;242m";
    const auto reset    = "\033[0m";

    const auto pannel_size = 25;

    auto empty_padding = [](int sz) -> std::string {
        std::stringstream ss;
        for (int pad = 0; pad <= sz; pad++)
            ss << (pad == sz ? " │" : " ");
        return ss.str();
    };

    output <<   "┌───────────────────┬───────────────────────────┐";

    for (int square = EnumSquare::a1; square <= EnumSquare::h8 ; square++) {
        // Prefix
        if (square % 8  == 0) output << "\n│ " + std::to_string(8-square/8) + " ";

        // Board and Pieces
        auto found = std::string();
        for (int color = White; color <= Black && found.empty(); color++)
            for (int piece = Pawns, idx = 0; piece <= King && !found[0]; piece++, idx++)
                if (Utils::GetSquare(Utils::Flip<Vertical>(pieces[piece] & pieces[color]), square))
                    found = (color ? fg_black : fg_white) + unicode[idx];
        output << (found.empty() ? std::string(fg_dim) + ". " : found + " ") << reset;

        // Information Pannel
        if ((square+1) % 8 == 0) output << "│ " << [&]() -> std::string {
            std::stringstream ss;
            switch(square/8) {
            case 0: ss << "TOPLAY: " << (to_play ? "BLACK" : "WHITE"); break;
            case 1: ss << "ENPASS: " << (en_passant > 0 ? SquareIndex[en_passant]: "-"); break;
            case 2: ss << "HMOVES: " << half_moves; break;
            case 3: ss << "FMOVES: " << full_moves; break;
            case 4: ss << "CASTLE: "
                       << (castling_rights[0] ? "K" : "-")
                       << (castling_rights[1] ? "Q" : "-")
                       << (castling_rights[2] ? "k" : "-")
                       << (castling_rights[3] ? "q" : "-"); break;
            default: return empty_padding(pannel_size);
            } return ss.str() + empty_padding(pannel_size - ss.str().size());
        }();

    }

    output << "\n│ ϴ a b c d e f g h │ " << empty_padding(pannel_size)
           << "\n└───────────────────┴───────────────────────────┘";
    return output.str();
}
