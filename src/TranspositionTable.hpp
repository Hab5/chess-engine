#pragma once

#include "Move.hpp"
#include "GameState.hpp"
#include "ChessEngine.hpp"
#include "ZobristHashing.hpp"

#include <iostream>
#include <random>
#include <cstring>

#define HASH_TABLE_SIZE 0x100000 * 64 // 32mb

enum TTFlag {
    HashExact,
    HashAlpha,
    HashBeta,
    // HashUnknown = 0xDEAD,
};

struct TTData {
    std::uint64_t hash;
    std::uint32_t flag;
    Move          move; // 1 byte
    std::int32_t  score;
    std::uint8_t  depth;
};

class TranspositionTable final {
public:

    inline auto Record(GameState& Board, int flag, int score, Move best, int depth) noexcept {
        TTData& Entry = table[Board.hash % HASH_TABLE_SIZE];

        if (Entry.hash == Board.hash && Entry.depth > depth)
            return;

        Entry.hash  = Board.hash;
        Entry.score = score;
        Entry.move  = best;
        Entry.flag  = flag;
        Entry.depth = depth;
    }

    inline auto Probe(GameState& Board, int alpha, int beta, int depth) noexcept {
        TTData& Entry = table[Board.hash % HASH_TABLE_SIZE];

        if (Entry.hash == Board.hash) {
            if (Entry.depth >= depth)
                return (Entry.flag == HashExact) ?  Entry.score :
                       (Entry.flag == HashAlpha  && Entry.score <= alpha) ? alpha :
                       (Entry.flag == HashBeta   && Entry.score >= beta ) ? beta  :
                       0xDEAD;
        } return 0xDEAD;
    }

    inline auto GetBestMove(GameState& Board) noexcept  {
        TTData& Entry = table[Board.hash % HASH_TABLE_SIZE];
        if (Entry.hash) return Entry.move;
        else return Move { EnumPiece(0), EnumSquare(0), EnumSquare(0), EnumMoveFlags(0) };
    }

    inline auto Clear() noexcept { std::memset(&table[0], 0, sizeof(table)); }

private:
    std::array<TTData, HASH_TABLE_SIZE> table { };
};
