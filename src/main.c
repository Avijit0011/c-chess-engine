#include "defs.h"
#include "bitboard.h"
#include "position.h"
#include "tt.h"
#include "uci.h"
#include "cli.h"
#include "perft.h"

int main(int argc, char *argv[]) {
    // 1. Initialize engine data structures
    init_zobrist();
    init_bitboards();
    init_tt(16); // Default 16 MB Hash table

    // 2. Parse mode flags
    if (argc > 1) {
        if (strcmp(argv[1], "uci") == 0 || strcmp(argv[1], "--uci") == 0) {
            uci_loop();
            free_tt();
            return 0;
        } else if (strcmp(argv[1], "perft") == 0 || strcmp(argv[1], "--perft") == 0) {
            perft_test_suite();
            free_tt();
            return 0;
        }
    }

    // 3. Default to Interactive CLI mode
    cli_loop();

    free_tt();
    return 0;
}
