#pragma once

#include "ChessEngine.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#include <string>
#include <array>
#include <sstream>
#include <algorithm>

class alignas(64) ChessBoard final {
    friend class  MoveGen;
    friend class  Perft;
    friend struct Move;
    friend class  FEN;
public:
    ChessBoard(const std::string& fen=STARTING_POSITION);

    friend std::ostream& operator<<(std::ostream& os, const ChessBoard& board) {
        return os << board.PrettyPrint();
    }

    [[nodiscard]] inline Bitboard& operator[](std::uint8_t query) noexcept {
        return pieces[query];
    }

    [[nodiscard]] inline Bitboard operator[](std::uint8_t query) const noexcept {
        return pieces[query];
    }

    [[nodiscard]] constexpr inline auto GetEnPassant() const noexcept {
        return en_passant;
    }

private:
    [[nodiscard]] std::string PrettyPrint() const noexcept;

    std::array<Bitboard, 8> pieces { };
    EnumColor      to_play;
    EnumSquare     en_passant;
    std::bitset<4> castling_rights;
    int            half_moves;
    int            full_moves;
};
