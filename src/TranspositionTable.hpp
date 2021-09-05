#pragma once

#include "ChessEngine.hpp"
#include "GameState.hpp"

#include <iostream>
#include <random>

namespace Generator {
    class ZobristKeys {
    public:
        [[nodiscard]] static auto Get() noexcept {
            struct _Zobrist {
                std::array<std::array<std::uint64_t, 64>, 12> Piece;
                std::array<std::uint64_t, 64>                 EnPassant;
                std::array<std::uint64_t, 16>                 Castle;
                std::uint64_t                                 Side;
            };

            return _Zobrist {
                .Piece     = ZobristKeys::PieceKeys(),
                .EnPassant = ZobristKeys::EnPassantKeys(),
                .Castle    = ZobristKeys::CastleKeys(),
                .Side      = rng64()
            };
        }

    private:
        [[nodiscard]] static auto PieceKeys() noexcept
            -> std::array<std::array<std::uint64_t, 64>, 12>  {
            std::array<std::array<std::uint64_t, 64>, 12> piece_keys { };
            for (int color = White; color <= Black; ++color)
                for (int piece = Pawns; piece <= King; ++piece)
                    for (EnumSquare square = a1; square <= h8; ++square)
                        piece_keys[piece+(6*color)-2][square] = rng64();
            return piece_keys;
        }

        [[nodiscard]] static auto EnPassantKeys() noexcept
            -> std::array<std::uint64_t, 64> {
            std::array<std::uint64_t, 64> en_passant_keys { };
            for (EnumSquare square = a1; square <= h8; ++square)
                en_passant_keys[square] = rng64();
            return en_passant_keys;
        }

        [[nodiscard]] static auto CastleKeys() noexcept
            -> std::array<std::uint64_t, 16> {
            std::array<std::uint64_t, 16> castle_keys { };
            for (int castle = 0; castle < 16; ++castle)
                castle_keys[castle] = rng64();
            return castle_keys;
        }

        static inline auto rng64 = std::mt19937_64(0xdeadbeef);

         ZobristKeys()=delete;
        ~ZobristKeys()=delete;
    };
}

class Zobrist final {
    friend struct Move;
public:
    [[nodiscard]] static inline auto Hash(GameState& Board) noexcept {
        std::uint64_t hash = 0ULL;

        if (Board.to_play == Black) hash ^= Keys.Side;

        for (int piece = Pawns; piece <= King; ++piece) {
            auto WhitePieces = Board[piece] & Board[White];
            auto BlackPieces = Board[piece] & Board[Black];

            while (WhitePieces) {
                auto square = Utils::PopLS1B(WhitePieces);
                hash ^= Keys.Piece[piece-2][square];
            }

            while (BlackPieces) {
                auto square = Utils::PopLS1B(BlackPieces);
                hash ^= Keys.Piece[piece+6-2][square];
            }

        }

        if (Board.en_passant) hash ^= Keys.EnPassant[Board.en_passant];
        hash ^= Keys.Castle[Board.castling_rights.to_ulong()];

        return hash;
    }

private:
     static const inline auto Keys = Generator::ZobristKeys::Get();

     Zobrist()=delete;
    ~Zobrist()=delete;
};

class TranspositionTable final {
public:

private:

     TranspositionTable()=delete;
    ~TranspositionTable()=delete;
};
