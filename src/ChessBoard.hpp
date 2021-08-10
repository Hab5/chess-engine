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
    friend class Move;
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

    [[nodiscard]] constexpr inline auto GetCastlingRights() const noexcept {
        return castling_rights;
    }

    EnumColor                    to_play;
private:
    [[nodiscard]] std::string PrettyPrint() const noexcept;

    std::array<Bitboard, 8> pieces { };

    std::bitset<4> castling_rights;
    EnumSquare     en_passant;
    int            half_moves;
    int            full_moves;

};
