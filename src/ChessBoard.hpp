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

    [[nodiscard]] friend std::ostream& operator<<(std::ostream& os, const ChessBoard& board) {
        return os << board.PrettyPrint();
    }

    [[nodiscard]] constexpr inline auto GetEnPassant() noexcept { return en_passant; }

private:
    [[nodiscard]] std::string PrettyPrint() const noexcept;

    std::array<std::uint64_t, 8> pieces { };
    std::bitset<4>               castling_rights;
    EnumSquare                   en_passant;
    bool                         to_play;
    int                          half_moves;
    int                          full_moves;

};
