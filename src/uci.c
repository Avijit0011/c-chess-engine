#include "uci.h"

static void parse_position(const char *command, Position *pos) {
    const char *p = command + 8; // Skip "position"

    if (strncmp(p, " startpos", 9) == 0) {
        parse_fen(pos, START_FEN);
    } else if (strncmp(p, " fen", 4) == 0) {
        p += 5;
        parse_fen(pos, p);
    } else {
        parse_fen(pos, START_FEN);
    }

    const char *moves_ptr = strstr(command, "moves");
    if (moves_ptr) {
        moves_ptr += 6;
        char move_buf[16];
        int idx = 0;

        while (*moves_ptr) {
            while (*moves_ptr == ' ') moves_ptr++;
            if (!*moves_ptr) break;

            idx = 0;
            while (*moves_ptr && *moves_ptr != ' ' && idx < 15) {
                move_buf[idx++] = *moves_ptr++;
            }
            move_buf[idx] = '\0';

            Move m = parse_move_string(move_buf, pos);
            if (m != 0) {
                make_move(pos, m);
            }
        }
    }
}

static void parse_go(const char *command, Position *pos) {
    SearchControl sc;
    memset(&sc, 0, sizeof(SearchControl));
    sc.depth_limit = 64;
    sc.time_limit_ms = -1;

    const char *p = strstr(command, "depth");
    if (p) {
        sc.depth_limit = atoi(p + 6);
    }

    p = strstr(command, "movetime");
    if (p) {
        sc.time_limit_ms = atoi(p + 9);
    }

    p = strstr(command, "wtime");
    int wtime = p ? atoi(p + 6) : -1;
    p = strstr(command, "btime");
    int btime = p ? atoi(p + 6) : -1;

    p = strstr(command, "winc");
    int winc = p ? atoi(p + 5) : 0;
    p = strstr(command, "binc");
    int binc = p ? atoi(p + 5) : 0;

    if (wtime != -1 && pos->side == WHITE) {
        sc.time_limit_ms = wtime / 25 + winc / 2;
    } else if (btime != -1 && pos->side == BLACK) {
        sc.time_limit_ms = btime / 25 + binc / 2;
    }

    p = strstr(command, "perft");
    if (p) {
        int depth = atoi(p + 6);
        perft_run(pos, depth);
        return;
    }

    Move best_move = search_position(pos, &sc);

    printf("bestmove ");
    if (best_move != 0) {
        print_move(best_move);
    } else {
        printf("0000");
    }
    printf("\n");
    fflush(stdout);
}

void uci_loop(void) {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    Position pos;
    parse_fen(&pos, START_FEN);

    char input[4096];

    while (1) {
        if (!fgets(input, sizeof(input), stdin)) break;

        // Strip trailing newline/carriage return
        size_t len = strlen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
            input[--len] = '\0';
        }

        if (strcmp(input, "uci") == 0) {
            printf("id name %s %s\n", ENGINE_NAME, ENGINE_VERSION);
            printf("id author %s\n", ENGINE_AUTHOR);
            printf("option name Hash type spin default 16 min 1 max 1024\n");
            printf("uciok\n");
            fflush(stdout);
        } else if (strcmp(input, "isready") == 0) {
            printf("readyok\n");
            fflush(stdout);
        } else if (strcmp(input, "ucinewgame") == 0) {
            clear_tt();
            parse_fen(&pos, START_FEN);
        } else if (strncmp(input, "position", 8) == 0) {
            parse_position(input, &pos);
        } else if (strncmp(input, "go", 2) == 0) {
            parse_go(input, &pos);
        } else if (strncmp(input, "setoption name Hash value ", 26) == 0) {
            int mb = atoi(input + 26);
            init_tt(mb);
        } else if (strcmp(input, "d") == 0) {
            print_board(&pos);
        } else if (strcmp(input, "quit") == 0) {
            break;
        }
    }
}
