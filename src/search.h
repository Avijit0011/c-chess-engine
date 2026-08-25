#ifndef SEARCH_H
#define SEARCH_H

#include "defs.h"
#include "position.h"
#include "bitboard.h"
#include "move.h"
#include "movegen.h"
#include "makemove.h"
#include "eval.h"
#include "tt.h"

typedef struct {
    uint64_t start_time;
    uint64_t stop_time;
    int depth_limit;
    int time_limit_ms;
    int quit_flag;
    int stop_search;
    uint64_t nodes;
} SearchControl;

uint64_t get_time_ms(void);

void reset_search_heuristics(void);
Move search_position(Position *pos, SearchControl *sc);

#endif // SEARCH_H
