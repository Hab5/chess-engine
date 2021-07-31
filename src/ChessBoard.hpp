#pragma once

#include "ChessEngine.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#include <string>
#include <array>
#include <sstream>
#include <algorithm>

class ChessBoard final {
friend class FEN;
public:
    ChessBoard(const std::string& fen=STARTING_POSITION);

    [[nodiscard]] inline std::uint64_t& operator[](std::size_t query) noexcept {
        return pieces[query];
    }

    [[nodiscard]] inline std::uint64_t operator[](std::size_t query) const noexcept {
        return pieces[query];
    }

    [[nodiscard]] friend std::ostream& operator<<(std::ostream& os, const ChessBoard& board) noexcept {
        return os << board.Show();
    }

private:
    [[nodiscard]] std::string Show() const noexcept;

    std::array<std::uint64_t, 8> pieces { };
    std::uint64_t                occupancy;
    std::bitset<4>               castling_rights;
    bool                         to_play;
    int                          en_passant;
    int                          half_moves;
    int                          full_moves;

};
