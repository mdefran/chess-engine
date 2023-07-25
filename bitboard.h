#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>

/*
Bitboards are 64-bit variables used to represent the location of pieces on the board.
Each bit corresponds to a square on the chessboard. This starts from the bottom right of the board, traversed in row-major order.
The presence of a piece on a square is indicated by a value of 1, while the absence of that piece is indicated by a 0.
*/
typedef uint64_t Bitboard;

// File masks
const Bitboard FILE_A = 0x8080808080808080ULL;
const Bitboard FILE_B = 0x4040404040404040ULL;
const Bitboard FILE_C = 0x2020202020202020ULL;
const Bitboard FILE_D = 0x1010101010101010ULL;
const Bitboard FILE_E = 0x0808080808080808ULL;
const Bitboard FILE_F = 0x0404040404040404ULL;
const Bitboard FILE_G = 0x0202020202020202ULL;
const Bitboard FILE_H = 0x0101010101010101ULL;

// Rank masks
const Bitboard RANK_1 = 0x00000000000000FFULL;
const Bitboard RANK_2 = 0x000000000000FF00ULL;
const Bitboard RANK_3 = 0x0000000000FF0000ULL;
const Bitboard RANK_4 = 0x00000000FF000000ULL;
const Bitboard RANK_5 = 0x000000FF00000000ULL;
const Bitboard RANK_6 = 0x0000FF0000000000ULL;
const Bitboard RANK_7 = 0x00FF000000000000ULL;
const Bitboard RANK_8 = 0xFF00000000000000ULL;

// General masks
const Bitboard EMPTY = 0ULL;
const Bitboard UNIVERSE = 0xFFFFFFFFFFFFFFFFULL;

// Set a bit at a given position to 1
#define SET_BIT(bitboard, bit) ((bitboard) |= (1ULL << bit))
// Set a bit at a given position to 0
#define CLEAR_BIT(bitboard, bit) ((bitboard) &= ~(1ULL << bit))
// Get the value of a bit at a given position
#define GET_BIT(bitboard, bit) ((bitboard) & (1ULL << bit))
// Get the number of trailing zeros after the least significant bit
#define GET_LSB(bitboard) (__builtin_ctzll(bitboard))
// Clear the LSB and return its index
inline int POP_LSB(Bitboard &bitboard) {
    int index = GET_LSB(bitboard);
    /*
    Clears the rightmost bit.
    When you subtract one from a binary number, the rightmost bit is set to 0, and all following bits are set to 1.
    Since the rightmost bit will not be set in bitboard - 1, it will be cleared when &= is applied.
    */
    bitboard &= bitboard - 1;
    return index;
}
#define BITBOARD(square) ((1ULL << square))

// Directional helper functions with boundary validation
constexpr Bitboard north(Bitboard bitboard) { return (bitboard & ~RANK_8) << 8; }
constexpr Bitboard east(Bitboard bitboard) { return (bitboard & ~FILE_H) >> 1; }
constexpr Bitboard south(Bitboard bitboard) { return (bitboard & ~RANK_1) >> 8; }
constexpr Bitboard west(Bitboard bitboard) { return (bitboard & ~FILE_A) << 1; }
constexpr Bitboard northeast(Bitboard bitboard) { return (bitboard & ~RANK_8 & ~FILE_H) << 7; }
constexpr Bitboard northwest(Bitboard bitboard) { return (bitboard & ~RANK_8 & ~FILE_A) << 9; }
constexpr Bitboard southwest(Bitboard bitboard) { return (bitboard & ~RANK_1 & ~FILE_A) >> 7; }
constexpr Bitboard southeast(Bitboard bitboard) { return (bitboard & ~RANK_1 & ~FILE_H) >> 9; }

#endif // BITBOARD_H