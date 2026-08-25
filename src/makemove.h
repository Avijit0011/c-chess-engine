#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "defs.h"
#include "position.h"
#include "movegen.h"

extern const int castling_rights_mask[64];

int make_move(Position *pos, Move move);
void unmake_move(Position *pos, Move move);

// Make null move (for search null-move pruning)
void make_null_move(Position *pos);
void unmake_null_move(Position *pos);

#endif // MAKEMOVE_H
