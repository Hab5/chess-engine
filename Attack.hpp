#pragma once

#include "ChessEngine.hpp"
#include "Utils.hpp"
#include "Magics.hpp"

#include <array>
#include <iostream>
#include <sys/types.h>


///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// ATTACK GENERATOR //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////// ATTACK GENERATOR  ///////////////////////////////////////

namespace Generator {

template <auto... Args>
class Attacks final { };

}

////////////////////////////////////// ATTACK DISPATCHER //////////////////////////////////////

template <auto... Args>
struct Attack final { };
























//////////////////////////////////////// FOR TESTS //////////////////////////////////////////

template <EnumPiece Piece>
struct SliderAttacks final { };

template <>
struct SliderAttacks<Bishops> final {
public:
    [[nodiscard]] static constexpr auto On(std::size_t square, std::uint64_t blockers) noexcept {
        std::uint64_t masks = 0ULL, mask = 0ULL;
        int target_rank = square / 8, target_file = square % 8; // 2D Square Index
        int r = target_rank+1, f = target_file+1;
        while (r <= 7 && f <= 7) { // NE
            mask = (1ULL << ((8*r++)+(f++)));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank+1, f = target_file-1;
        while (r <= 7 && f >= 0) { // NE
            mask = (1ULL << ((8*r++)+(f--)));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank-1, f = target_file+1;
        while (r >= 0 && f <= 7) { // SE
            mask = (1ULL << ((8*r--)+(f++)));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank-1, f = target_file-1;
        while (r >= 0 && f >= 0) { // SE
            mask = (1ULL << ((8*r--)+(f--)));
            masks |= mask; if (mask & blockers) break;
        }
        return masks;
    }
};

template <>
struct SliderAttacks<Rooks> final {
public:
    [[nodiscard]] static constexpr auto On(std::size_t square, std::uint64_t blockers) noexcept {
        std::uint64_t masks = 0ULL, mask = 0ULL;
        int target_rank = square / 8, target_file = square % 8; // 2D Square Index
        int r = target_rank+1, f = target_file;
        while (r <= 7) { // N
            mask = (1ULL << ((8*r++)+f));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank-1, f = target_file;
        while (r >= 0) { // S
            mask = (1ULL << ((8*r--)+f));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank, f = target_file+1;
        while (f <= 7) { // E
            mask = (1ULL << (8*r+(f++)));
            masks |= mask; if (mask & blockers) break;
        } r = target_rank, f = target_file-1;
        while (f >= 0) { // W
            mask = (1ULL << (8*r+(f--)));
            masks |= mask; if (mask & blockers) break;
        }
        return masks;
    }
};
