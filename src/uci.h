#ifndef UCI_H
#define UCI_H

#include "defs.h"
#include "position.h"
#include "move.h"
#include "movegen.h"
#include "makemove.h"
#include "search.h"
#include "tt.h"
#include "perft.h"

void uci_loop(void);

#endif // UCI_H
