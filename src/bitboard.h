#ifndef BITBOARD_H
#define BITBOARD_H

#include "defs.h"

// File bitmask constants to avoid board edge wrap-around
#define NOT_A_FILE  0xFeFeFeFeFeFeFeFeULL
#define NOT_H_FILE  0x7f7f7f7f7f7f7f7fULL
#define NOT_AB_FILE 0xfcfcfcfcfcfcfcfcULL
#define NOT_HG_FILE 0x3f3f3f3f3f3f3f3fULL

// Bitboard manipulation macros
#define SET_BIT(bb, square) ((bb) |= (1ULL << (square)))
#define GET_BIT(bb, square) (((bb) & (1ULL << (square))) ? 1 : 0)
#define POP_BIT(bb, square) ((bb) &= ~(1ULL << (square)))

// Attack tables
extern Bitboard pawn_attacks[2][64];
extern Bitboard knight_attacks[64];
extern Bitboard king_attacks[64];

// Bitboard utility functions
int count_bits(Bitboard bb);
int get_lsb(Bitboard bb);

// Initialization
void init_bitboards(void);

// Attack Generators
Bitboard mask_pawn_attacks(int side, int square);
Bitboard mask_knight_attacks(int square);
Bitboard mask_king_attacks(int square);

Bitboard get_bishop_attacks(int square, Bitboard occupancy);
Bitboard get_rook_attacks(int square, Bitboard occupancy);
Bitboard get_queen_attacks(int square, Bitboard occupancy);

// Print bitboard representation for debugging
void print_bitboard(Bitboard bb);

#endif // BITBOARD_H
