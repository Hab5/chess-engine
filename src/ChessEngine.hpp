#pragma once

#include <cstdint>
#include <array>
#include <string>

enum EnumColor: std::uint8_t {
    White = 0x00,
    Black = 0x01
};

enum EnumPiece: std::uint8_t {
    Pawns   = 0x02,
    Knights = 0x03,
    Bishops = 0x04,
    Rooks   = 0x05,
    Queens  = 0x06,
    King    = 0x07
};

enum EnumSquare: std::uint8_t {
    a1,b1,c1,d1,e1,f1,g1,h1,
    a2,b2,c2,d2,e2,f2,g2,h2,
    a3,b3,c3,d3,e3,f3,g3,h3,
    a4,b4,c4,d4,e4,f4,g4,h4,
    a5,b5,c5,d5,e5,f5,g5,h5,
    a6,b6,c6,d6,e6,f6,g6,h6,
    a7,b7,c7,d7,e7,f7,g7,h7,
    a8,b8,c8,d8,e8,f8,g8,h8
};

constexpr std::array<const char*, 65> SquareIndex {
    "a1","b1","c1","d1","e1","f1","g1","h1",
    "a2","b2","c2","d2","e2","f2","g2","h2",
    "a3","b3","c3","d3","e3","f3","g3","h3",
    "a4","b4","c4","d4","e4","f4","g4","h4",
    "a5","b5","c5","d5","e5","f5","g5","h5",
    "a6","b6","c6","d6","e6","f6","g6","h6",
    "a7","b7","c7","d7","e7","f7","g7","h7",
    "a8","b8","c8","d8","e8","f8","g8","h8",
    "None"
};

enum EnumSquareColor: std::uint64_t {
    LightSquare = 0x55AA55AA55AA55AA,
    DarkSquare  = 0xAA55AA55AA55AA55
};

enum EnumFile: std::uint64_t {
    File_A = 0x0101010101010101,
    File_B = 0x0202020202020202,
    File_C = 0x0404040404040404,
    File_D = 0x0808080808080808,
    File_E = 0x1010101010101010,
    File_F = 0x2020202020202020,
    File_G = 0x4040404040404040,
    File_H = 0x8080808080808080
};

enum EnumRank: std::uint64_t {
    Rank_1 = 0x00000000000000ff,
    Rank_2 = 0x000000000000ff00,
    Rank_3 = 0x0000000000ff0000,
    Rank_4 = 0x00000000ff000000,
    Rank_5 = 0x000000ff00000000,
    Rank_6 = 0x0000ff0000000000,
    Rank_7 = 0x00ff000000000000,
    Rank_8 = 0xff00000000000000
};


///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// ENUMSQUARE OPERATOR OVERLOAD /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

inline std::ostream& operator<<(std::ostream& os, const EnumSquare& square) {
    return (os << std::string(SquareIndex[square]));
}

inline constexpr EnumSquare& operator++(EnumSquare& square) noexcept {
    return square = static_cast<EnumSquare>(static_cast<int>(square)+1);
}

inline constexpr EnumSquare operator++(EnumSquare& square, int) noexcept {
  EnumSquare tmp = square; ++square;
  return tmp;
}

inline constexpr EnumSquare& operator--(EnumSquare& square) noexcept {
    return square = static_cast<EnumSquare>(static_cast<int>(square)-1);
}

inline constexpr EnumSquare operator--(EnumSquare& square, int) noexcept {
  EnumSquare tmp = square; --square;
  return tmp;
}

inline constexpr EnumSquare operator%(EnumSquare square, int rhs) noexcept {
    return static_cast<EnumSquare>(static_cast<int>(square)%rhs);
}

inline constexpr EnumSquare operator+(EnumSquare square, int rhs) noexcept {
    return static_cast<EnumSquare>(static_cast<int>(square)+rhs);
}

inline constexpr EnumSquare operator-(EnumSquare square, int rhs) noexcept {
    return static_cast<EnumSquare>(static_cast<int>(square)-rhs);
}

inline constexpr EnumSquare& operator+=(EnumSquare& square, int rhs) noexcept {
    return square = square + rhs;
}

inline constexpr EnumSquare& operator-=(EnumSquare& square, int rhs) noexcept {
    return square = square - rhs;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// BITBOARD OPERATOR OVERLOAD ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

using Bitboard = std::uint64_t;

constexpr inline Bitboard& operator|=(Bitboard& bitboard, EnumSquare square) noexcept {
    return bitboard |= (0ULL | (1ULL << square));
}

constexpr inline Bitboard& operator^=(Bitboard& bitboard, EnumSquare square) noexcept {
    return bitboard ^= (0ULL | (1ULL << square));
}

constexpr inline Bitboard operator&(Bitboard bitboard, EnumSquare square) noexcept {
    return bitboard & (0ULL | (1ULL << square));
}

constexpr inline Bitboard operator&(EnumSquare square, Bitboard bitboard) noexcept {
    return bitboard & square;
}

constexpr inline Bitboard operator|(Bitboard bitboard, EnumSquare square) noexcept {
    return bitboard | (0ULL | (1ULL << square));
}

constexpr inline Bitboard operator|(EnumSquare square, Bitboard bitboard) noexcept {
    return bitboard | square;
}

constexpr inline Bitboard operator^(Bitboard bitboard, EnumSquare square) noexcept {
    return bitboard ^ (0ULL | (1ULL << square));
}

constexpr inline Bitboard operator^(EnumSquare square, Bitboard bitboard) noexcept {
    return bitboard ^ square;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
