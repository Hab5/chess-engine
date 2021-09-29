#include "ZobristHashing.hpp"
#include "GameState.hpp"
#include "FEN.hpp"

GameState::GameState(const std::string& fen) {
    FEN::Load(fen, *this);
    hash = ZobristHashing::Hash(*this);
}

#define DEBUG_UNICODE

[[nodiscard]] std::string GameState::PrettyPrint() noexcept {
    std::stringstream output;

    #if defined(DEBUG_UNICODE)
    const std::array<std::string, 12> sym {
        "♙", "♘", "♗", "♖", "♕", "♔",
        "♟", "♞", "♝", "♜", "♛", "♚"
    };

    #else
    const std::array<std::string, 12> sym {
        "P", "N", "B", "R", "Q", "K",
        "p", "n", "b", "r", "q", "k"
    };
    #endif

    const auto pannel_size = 28;

    auto empty_padding = [](int sz) -> std::string {
        std::stringstream ss;
        for (int pad = 0; pad <= sz; pad++)
            ss << (pad == sz ? " │" : " ");
        return ss.str();
    };

    output <<   "┌───────────────────┬──────────────────────────────┐";
    for (EnumSquare square = a1; square <= h8 ; ++square) {
        if (square % 8  == 0) output << "\n│ " + std::to_string(8-square/8) + " ";
        auto found = std::string();
        for (int color = White; color <= Black && found.empty(); color++)
            for (int piece = Pawns, idx = 0; piece <= King && !found[0]; piece++, idx++)
                if (Utils::Flip<Vertical>(pieces[piece] & pieces[color]) & square)
                    found = sym[idx+(color == White ? 0:6)];
        output << (found.empty() ? ". " : found + " ");
        if ((square+1) % 8 == 0) output << "│ " << [&]() -> std::string {
            std::stringstream ss;
            switch(square/8) {
            case 0: ss << "ZKHASH: " << hash; break;
            case 1: ss << "TOPLAY: " << (to_play ? "BLACK" : "WHITE"); break;
            case 3: ss << "ENPASS: " << (en_passant ? SquareStr[en_passant]: "-"); break;
            case 4: ss << "FMOVES: " << full_moves; break;
            case 5: ss << "HMOVES: " << half_moves; break;
            case 2: ss << "CASTLE: "
                       << (castling_rights[0] ? "K" : "-")
                       << (castling_rights[1] ? "Q" : "-")
                       << (castling_rights[2] ? "k" : "-")
                       << (castling_rights[3] ? "q" : "-"); break;
            default: return empty_padding(pannel_size);
            } return ss.str() + empty_padding(pannel_size - ss.str().size());
        }();
    }
    output << "\n│ ϴ a b c d e f g h │ " << empty_padding(pannel_size)
           << "\n└───────────────────┴──────────────────────────────┘";
    return output.str();
}
