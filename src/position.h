#ifndef POSITION_H
#define POSITION_H

#include "defs.h"
#include "bitboard.h"

// Zobrist hash keys
extern uint64_t piece_keys[12][64];
extern uint64_t enpassant_keys[64];
extern uint64_t castling_keys[16];
extern uint64_t side_key;

// Character representation of pieces
extern const char piece_ascii[13];
extern const char *piece_unicode[13];

// Initialization
void init_zobrist(void);
uint64_t generate_hash_key(const Position *pos);

// Position management
void reset_position(Position *pos);
int parse_fen(Position *pos, const char *fen);
void generate_fen(const Position *pos, char *fen);
void print_board(const Position *pos);

// Utility functions
int char_to_piece(char c);
char square_to_file(int sq);
char square_to_rank(int sq);

#endif // POSITION_H
