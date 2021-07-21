#include "Utils.hpp"

namespace Utils {

template <>
[[nodiscard]] constexpr std::uint64_t Flip<Vertical>(std::uint64_t bitboard) noexcept {
    return __builtin_bswap64(bitboard);
}

template <>
[[nodiscard]] constexpr std::uint64_t Flip<Horizontal>(std::uint64_t bitboard) noexcept {
        const std::uint64_t k1 = 0x5555555555555555;
        const std::uint64_t k2 = 0x3333333333333333;
        const std::uint64_t k4 = 0x0f0f0f0f0f0f0f0f;
        bitboard = ((bitboard >> 1) & k1) +  2*(bitboard & k1);
        bitboard = ((bitboard >> 2) & k2) +  4*(bitboard & k2);
        bitboard = ((bitboard >> 4) & k4) + 16*(bitboard & k4);
        return bitboard;
}

template <>
[[nodiscard]] constexpr std::uint64_t Flip<Diagonal>(std::uint64_t bitboard) noexcept {
    return bitboard; // TODO
}

void Print(std::uint64_t bitboard) noexcept {
    auto bitset = std::bitset<64>(Flip<Vertical>(bitboard));
    for (int i = 0; i < 64;) {
        std::cout << (i % 8 ? "" : "  " + std::to_string(8-i/8) + " ")
                  << (bitset[i] ? "■ " : "□ "); i++;
        if (i % 8 == 0) {
            if      (i/8 == 1) std::cout << std::hex << "┃ HEX: " << bitset.to_ullong();
            else if (i/8 == 2) std::cout << std::dec << "┃ DEC: " << bitset.to_ullong();
            else std::cout << "┃";
            std::cout << std::dec << '\n';
        }
    } std::cout << "    a b c d e f g h \n\n";
}


template <EnumSquare Square>
[[nodiscard]] constexpr bool GetSquare(std::uint64_t bitboard) noexcept {
    return bitboard & (1ULL << Square);
}

template <EnumSquare Square>
[[nodiscard]] static constexpr auto SetSquare(std::uint64_t bitboard) noexcept {
    return bitboard |= (1ULL << Square);
}

template <EnumSquare Square>
[[nodiscard]] constexpr auto PopSquare(std::uint64_t bitboard) noexcept {
    if (bitboard  & (1ULL << Square))
        bitboard ^= (1ULL << Square);
    return bitboard;
}

template <EnumSquare Square>
[[nodiscard]] constexpr auto MakeSquare() noexcept {
    return 0ULL | (1ULL << Square);
}

}
