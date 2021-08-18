#include "GameState.hpp"
#include "FEN.hpp"

GameState::GameState(const std::string& fen) { FEN::Load(fen, *this); }

[[nodiscard]] std::string GameState::PrettyPrint() const noexcept {
    std::stringstream output;

    const std::array<std::string, 6> unicode { "♟", "♞", "♝", "♜", "♛", "♚" };

    const auto fg_white = "\033[38;5;246m";
    const auto fg_black = "\033[38;5;241m";
    const auto fg_dim   = "\033[38;5;242m";
    const auto reset    = "\033[0m";

    const auto pannel_size = 14;

    auto empty_padding = [](int sz) -> std::string {
        std::stringstream ss;
        for (int pad = 0; pad <= sz; pad++)
            ss << (pad == sz ? " │" : " ");
        return ss.str();
    };

    output <<   "┌───────────────────┬────────────────┐";
    for (EnumSquare square = a1; square <= h8 ; ++square) {
        if (square % 8  == 0) output << "\n│ " + std::to_string(8-square/8) + " ";
        auto found = std::string();
        for (int color = White; color <= Black && found.empty(); color++)
            for (int piece = Pawns, idx = 0; piece <= King && !found[0]; piece++, idx++)
                if (Utils::Flip<Vertical>(pieces[piece] & pieces[color]) & square)
                    found = (color ? fg_black : fg_white) + unicode[idx];
        output << (found.empty() ? std::string(fg_dim) + ". " : found + " ") << reset;
        if ((square+1) % 8 == 0) output << "│ " << [&]() -> std::string {
            std::stringstream ss;
            switch(square/8) {
            case 0: ss << "TOPLAY: " << (to_play ? "BLACK" : "WHITE"); break;
            case 2: ss << "ENPASS: " << (en_passant ? SquareStr[en_passant]: "-"); break;
            case 3: ss << "FMOVES: " << full_moves; break;
            case 4: ss << "HMOVES: " << half_moves; break;
            case 1: ss << "CASTLE: "
                       << (castling_rights[0] ? "K" : "-")
                       << (castling_rights[1] ? "Q" : "-")
                       << (castling_rights[2] ? "k" : "-")
                       << (castling_rights[3] ? "q" : "-"); break;
            default: return empty_padding(pannel_size);
            } return ss.str() + empty_padding(pannel_size - ss.str().size());
        }();
    }
    output << "\n│ ϴ a b c d e f g h │ " << empty_padding(pannel_size)
           << "\n└───────────────────┴────────────────┘";
    return output.str();
}
