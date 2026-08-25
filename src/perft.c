#include "perft.h"
#include "search.h"

uint64_t perft_driver(Position *pos, int depth) {
    if (depth == 0) return 1ULL;

    uint64_t nodes = 0ULL;
    MoveList list;
    generate_moves(pos, &list);

    for (int i = 0; i < list.count; i++) {
        Move m = list.moves[i];
        if (!make_move(pos, m)) continue;

        nodes += perft_driver(pos, depth - 1);
        unmake_move(pos, m);
    }

    return nodes;
}

void perft_run(Position *pos, int depth) {
    printf("\n --- Starting Perft Test (Depth %d) ---\n\n", depth);

    uint64_t start = get_time_ms();
    uint64_t total_nodes = 0ULL;

    MoveList list;
    generate_moves(pos, &list);

    for (int i = 0; i < list.count; i++) {
        Move m = list.moves[i];
        if (!make_move(pos, m)) continue;

        uint64_t nodes = perft_driver(pos, depth - 1);
        total_nodes += nodes;
        unmake_move(pos, m);

        printf(" ");
        print_move(m);
        printf(": %llu\n", (unsigned long long)nodes);
    }

    uint64_t elapsed = get_time_ms() - start;
    if (elapsed == 0) elapsed = 1;
    uint64_t nps = (total_nodes * 1000) / elapsed;

    printf("\n Total Nodes: %llu\n", (unsigned long long)total_nodes);
    printf(" Time Elapsed: %llu ms\n", (unsigned long long)elapsed);
    printf(" NPS:          %llu\n\n", (unsigned long long)nps);
}

void perft_test_suite(void) {
    Position pos;
    printf("\n ==========================================\n");
    printf("      RUNNING CHESS PERFT TEST SUITE       \n");
    printf(" ==========================================\n");

    // 1. Startpos Depth 5
    parse_fen(&pos, START_FEN);
    printf("\n[Test 1] Startpos (Depth 4)\nExpected: 197281\n");
    perft_run(&pos, 4);

    // 2. Kiwipete Depth 3
    parse_fen(&pos, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    printf("\n[Test 2] Kiwipete Position (Depth 3)\nExpected: 97862\n");
    perft_run(&pos, 3);

    // 3. Position 3 Depth 4
    parse_fen(&pos, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    printf("\n[Test 3] Position 3 (Depth 4)\nExpected: 43238\n");
    perft_run(&pos, 4);
}
