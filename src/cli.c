#include "cli.h"
#include "uci.h"

static void print_banner(void) {
    printf("\n");
    printf(" ==========================================================================\n");
    printf("   ____ _   _ _____ ____ ____  _____ _  _ ____ ___ _  _ _____\n");
    printf("  / ___| | | | ____/ ___/ ___||  ___| \\| |  _ \\_ _| \\| | ____|\n");
    printf(" | |   | |_| |  _| \\___ \\___ \\| |_  |  ` | |_) | ||  ` |  _|\n");
    printf(" | |___|  _  | |___ ___) |__) |  _| | |\\  |  _ <| || |\\  | |___\n");
    printf("  \\____|_| |_|_____|____/____/|_|   |_| \\_|_| \\_\\___|_| \\_|_____|\n");
    printf("                                                       \n");
    printf("         %s v%s - C Bitboard Engine (%s)\n", ENGINE_NAME, ENGINE_VERSION, ENGINE_AUTHOR);
    printf(" ==========================================================================\n");
    printf(" Type 'help' for a list of available CLI commands.\n\n");
}

static void print_help(void) {
    printf("\n --- Available CLI Commands ---\n");
    printf("  <move>        : Make move in algebraic notation (e.g. e2e4, g1f3, e7e8q)\n");
    printf("  go [depth]    : Engine calculates and plays the best move (default depth: 8)\n");
    printf("  eval          : Print evaluation score for current position\n");
    printf("  fen <string>  : Set position from FEN string\n");
    printf("  start         : Reset board to standard initial position\n");
    printf("  d / display   : Print board visual diagram & FEN\n");
    printf("  undo          : Undo the last move played\n");
    printf("  auto [moves]  : Run engine self-play mode\n");
    printf("  perft <depth> : Run move generator performance test\n");
    printf("  uci           : Switch to UCI protocol mode\n");
    printf("  quit / exit   : Exit program\n\n");
}

void cli_loop(void) {
    Position pos;
    parse_fen(&pos, START_FEN);

    print_banner();
    print_board(&pos);

    char input[1024];

    while (1) {
        printf("ChessEngine> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) break;

        size_t len = strlen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
            input[--len] = '\0';
        }

        if (len == 0) continue;

        if (strcmp(input, "help") == 0) {
            print_help();
        } else if (strcmp(input, "d") == 0 || strcmp(input, "display") == 0) {
            print_board(&pos);
        } else if (strcmp(input, "start") == 0) {
            parse_fen(&pos, START_FEN);
            print_board(&pos);
        } else if (strncmp(input, "fen ", 4) == 0) {
            if (parse_fen(&pos, input + 4)) {
                print_board(&pos);
            } else {
                printf(" Invalid FEN string!\n");
            }
        } else if (strcmp(input, "eval") == 0) {
            int score = evaluate(&pos);
            printf("\n Evaluation Score: %d cp (%s perspective)\n\n",
                score, (pos.side == WHITE) ? "White" : "Black");
        } else if (strncmp(input, "go", 2) == 0) {
            int depth = 8;
            if (strlen(input) > 3) {
                depth = atoi(input + 3);
                if (depth <= 0) depth = 8;
            }

            SearchControl sc;
            memset(&sc, 0, sizeof(SearchControl));
            sc.depth_limit = depth;
            sc.time_limit_ms = -1;

            printf(" Searching at depth %d...\n", depth);
            Move best = search_position(&pos, &sc);

            if (best != 0) {
                printf("\n Engine plays move: ");
                print_move(best);
                printf("\n\n");
                make_move(&pos, best);
                print_board(&pos);
            } else {
                printf(" No legal move available!\n");
            }
        } else if (strcmp(input, "undo") == 0) {
            if (pos.his_ply > 0) {
                Move last = pos.history[pos.his_ply - 1].move;
                unmake_move(&pos, last);
                printf(" Undid move: ");
                print_move(last);
                printf("\n");
                print_board(&pos);
            } else {
                printf(" No moves to undo!\n");
            }
        } else if (strncmp(input, "perft ", 6) == 0) {
            int depth = atoi(input + 6);
            if (depth > 0) {
                perft_run(&pos, depth);
            }
        } else if (strncmp(input, "auto", 4) == 0) {
            int max_moves = 40;
            if (strlen(input) > 5) {
                max_moves = atoi(input + 5);
                if (max_moves <= 0) max_moves = 40;
            }

            printf("\n Starting Self-Play (Max %d moves)...\n", max_moves);
            for (int m = 0; m < max_moves; m++) {
                SearchControl sc;
                memset(&sc, 0, sizeof(SearchControl));
                sc.depth_limit = 6;
                sc.time_limit_ms = -1;

                Move best = search_position(&pos, &sc);
                if (best == 0) {
                    printf(" Game over! No legal moves.\n");
                    break;
                }

                printf(" Move %d (%s): ", m + 1, (pos.side == WHITE) ? "White" : "Black");
                print_move(best);
                printf("\n");

                make_move(&pos, best);
                print_board(&pos);
            }
        } else if (strcmp(input, "uci") == 0) {
            printf(" Switching to UCI mode...\n");
            uci_loop();
            break;
        } else if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            printf(" Goodbye!\n");
            break;
        } else {
            // Try parsing move string directly (e.g. e2e4)
            Move m = parse_move_string(input, &pos);
            if (m != 0) {
                if (make_move(&pos, m)) {
                    print_board(&pos);
                } else {
                    printf(" Illegal move (leaves king in check)!\n");
                }
            } else {
                printf(" Unknown command or invalid move: '%s'\n", input);
                printf(" Type 'help' for instructions.\n");
            }
        }
    }
}
