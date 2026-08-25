#ifndef EVAL_H
#define EVAL_H

#include "defs.h"
#include "position.h"
#include "bitboard.h"

// Piece values
extern const int piece_value[13];

// Main evaluation function (returns score from side to move perspective)
int evaluate(const Position *pos);

#endif // EVAL_H
