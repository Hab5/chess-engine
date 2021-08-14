#pragma once

#include "ChessEngine.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#include <string>
#include <array>
#include <sstream>
#include <algorithm>

class ChessBoard final {
    friend class  MoveGen;
    friend struct Move;
    friend class  FEN;
public:
    ChessBoard(const std::string& fen=STARTING_POSITION);

    friend std::ostream& operator<<(std::ostream& os, const ChessBoard& board) {
        return os << board.PrettyPrint();
    }

    [[nodiscard]] inline Bitboard& operator[](std::size_t query) noexcept {
        return pieces[query];
    }

    [[nodiscard]] inline Bitboard operator[](std::size_t query) const noexcept {
        return pieces[query];
    }

    [[nodiscard]] constexpr inline auto GetEnPassant() const noexcept {
        return en_passant;
    }

private:
    [[nodiscard]] std::string PrettyPrint() const noexcept;

    EnumColor      to_play;
    EnumSquare     en_passant;
    int            half_moves;
    int            full_moves;
    std::bitset<4> castling_rights;
    std::array<Bitboard, 8> pieces { };
};
