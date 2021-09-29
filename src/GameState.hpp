#pragma once

#include "ChessEngine.hpp"
#include "GetAttack.hpp"
#include "Utils.hpp"
#include "FEN.hpp"

#include <string>
#include <array>
#include <sstream>
#include <algorithm>

class GameState final {
    friend class  TranspositionTable;
    friend class  ZobristHashing;
    friend class  MoveGeneration;
    friend class  Evaluation;
    friend class  Search;
    friend class  Perft;
    friend struct Move;
    friend class  FEN;
    friend class  UCI;

public:
    GameState(const std::string& fen=STARTING_POSITION);

    template <EnumColor Color> [[nodiscard]]
    static inline auto InCheck(GameState& Board, EnumSquare square) noexcept {
        const auto enemies   = Board[~Color];
        const auto occupancy = Board[ Color] | enemies;

        #define Pp    (Board[Pawns  ]                ) & enemies
        #define Nn    (Board[Knights]                ) & enemies
        #define BbQq  (Board[Bishops] | Board[Queens]) & enemies
        #define RrQq  (Board[Rooks  ] | Board[Queens]) & enemies
        #define Kk_   (Board[King   ]                ) & enemies

        if      (GetAttack<Color, Pawns>::On(square)            & Pp  ) return true;
        else if (GetAttack<Knights     >::On(square)            & Nn  ) return true;
        else if (GetAttack<Bishops     >::On(square, occupancy) & BbQq) return true;
        else if (GetAttack<Rooks       >::On(square, occupancy) & RrQq) return true;
        else if (GetAttack<King        >::On(square)            & Kk_ ) return true;
        else return false;
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

    friend std::ostream& operator<<(std::ostream& os, GameState& board) {
        return os << board.PrettyPrint();
    }

private:
    [[nodiscard]] std::string PrettyPrint() noexcept;

    std::array<Bitboard, 8> pieces { };

    std::uint64_t  hash;
    EnumColor      to_play;
    EnumSquare     en_passant;
    std::bitset<4> castling_rights;
    int            half_moves;
    int            full_moves;
};
