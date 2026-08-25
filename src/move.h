#ifndef MOVE_H
#define MOVE_H

#include "defs.h"
#include "position.h"

// Move string formatting
void print_move(Move move);
void sprintf_move(Move move, char *str);

// Move parsing
Move parse_move_string(const char *move_str, const Position *pos);

// Move list helper
void add_move(MoveList *list, Move move);

#endif // MOVE_H
