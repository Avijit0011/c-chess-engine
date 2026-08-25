#ifndef PERFT_H
#define PERFT_H

#include "defs.h"
#include "position.h"
#include "move.h"
#include "movegen.h"
#include "makemove.h"

uint64_t perft_driver(Position *pos, int depth);
void perft_run(Position *pos, int depth);
void perft_test_suite(void);

#endif // PERFT_H
