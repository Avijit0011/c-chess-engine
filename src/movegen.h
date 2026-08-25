#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"
#include "bitboard.h"

// Attack checking & Check detector
int is_square_attacked(const Position *pos, int square, int attacker_side);
int in_check(const Position *pos, int side);

// Move generators
void generate_moves(const Position *pos, MoveList *list);
void generate_captures(const Position *pos, MoveList *list);

#endif // MOVEGEN_H
