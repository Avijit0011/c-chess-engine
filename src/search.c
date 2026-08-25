#include "search.h"

#ifdef _WIN32
#include <windows.h>
uint64_t get_time_ms(void) {
    return GetTickCount();
}
#else
#include <sys/time.h>
uint64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

// Search Heuristic Tables
static Move killer_moves[2][MAX_PLY];
static int history_moves[13][64];

// MVV-LVA (Most Valuable Victim - Least Valuable Attacker) table
static const int mvv_lva[13][13] = {
    // Victim: EMPTY, P, N, B, R, Q, K, p, n, b, r, q, k
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // Attacker P
    { 0, 105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605 },
    // Attacker N
    { 0, 104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604 },
    // Attacker B
    { 0, 103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603 },
    // Attacker R
    { 0, 102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602 },
    // Attacker Q
    { 0, 101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601 },
    // Attacker K
    { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 },

    // Black pieces
    { 0, 105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605 },
    { 0, 104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604 },
    { 0, 103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603 },
    { 0, 102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602 },
    { 0, 101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601 },
    { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 },
};

void reset_search_heuristics(void) {
    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_moves, 0, sizeof(history_moves));
}

static void check_time(SearchControl *sc) {
    if (sc->time_limit_ms != -1 && get_time_ms() >= sc->stop_time) {
        sc->stop_search = 1;
    }
}

static int score_move(Position *pos, Move move, Move tt_move) {
    if (move == tt_move) return 2000000;

    if (MOVE_CAPTURE(move)) {
        int attacker = MOVE_PIECE(move);
        int victim = pos->board[MOVE_TO(move)];
        if (MOVE_ENPASSANT(move)) {
            victim = (pos->side == WHITE) ? p : P;
        }
        return 1000000 + mvv_lva[attacker][victim];
    } else {
        if (pos->ply < MAX_PLY) {
            if (killer_moves[0][pos->ply] == move) return 900000;
            if (killer_moves[1][pos->ply] == move) return 800000;
        }
        return history_moves[MOVE_PIECE(move)][MOVE_TO(move)];
    }
}

static void sort_moves(Position *pos, MoveList *list, Move tt_move) {
    int scores[MAX_MOVES];
    for (int i = 0; i < list->count; i++) {
        scores[i] = score_move(pos, list->moves[i], tt_move);
    }

    for (int i = 0; i < list->count - 1; i++) {
        for (int j = i + 1; j < list->count; j++) {
            if (scores[j] > scores[i]) {
                int temp_score = scores[i];
                scores[i] = scores[j];
                scores[j] = temp_score;

                Move temp_move = list->moves[i];
                list->moves[i] = list->moves[j];
                list->moves[j] = temp_move;
            }
        }
    }
}

static int quiescence(Position *pos, int alpha, int beta, SearchControl *sc) {
    sc->nodes++;
    if ((sc->nodes & 2047) == 0) check_time(sc);
    if (sc->stop_search) return 0;

    if (pos->ply >= MAX_PLY - 1) return evaluate(pos);

    int stand_pat = evaluate(pos);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    MoveList list;
    generate_captures(pos, &list);
    sort_moves(pos, &list, 0);

    for (int i = 0; i < list.count; i++) {
        Move m = list.moves[i];
        if (!make_move(pos, m)) continue;

        int score = -quiescence(pos, -beta, -alpha, sc);
        unmake_move(pos, m);

        if (sc->stop_search) return 0;

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

static int negamax(Position *pos, int alpha, int beta, int depth, SearchControl *sc) {
    sc->nodes++;
    if ((sc->nodes & 2047) == 0) check_time(sc);
    if (sc->stop_search) return 0;

    if (pos->ply >= MAX_PLY - 1) return evaluate(pos);

    int is_root = (pos->ply == 0);
    int in_chk = in_check(pos, pos->side);
    if (in_chk && depth < 32) depth++;

    if (depth <= 0) return quiescence(pos, alpha, beta, sc);

    // 50-move rule / repetition check
    if (!is_root && (pos->fifty_move >= 100)) return 0;

    // Transposition Table lookup
    Move tt_move = 0;
    int tt_score = read_tt(pos->hash_key, depth, alpha, beta, &tt_move, pos->ply);
    if (!is_root && tt_score != 100000) return tt_score;

    // Null Move Pruning
    if (depth >= 3 && !in_chk && !is_root && (pos->occupancy[pos->side] & ~pos->bitboards[(pos->side == WHITE ? P : p) - 1])) {
        make_null_move(pos);
        int null_score = -negamax(pos, -beta, -beta + 1, depth - 1 - 2, sc);
        unmake_null_move(pos);

        if (sc->stop_search) return 0;
        if (null_score >= beta) return beta;
    }

    MoveList list;
    generate_moves(pos, &list);
    sort_moves(pos, &list, tt_move);

    int legal_moves = 0;
    Move best_move_found = 0;
    int hash_flag = HASH_FLAG_ALPHA;

    for (int i = 0; i < list.count; i++) {
        Move m = list.moves[i];
        if (!make_move(pos, m)) continue;

        legal_moves++;
        int score;

        // Late Move Reduction (LMR)
        if (legal_moves > 4 && depth >= 3 && !in_chk && !MOVE_CAPTURE(m) && MOVE_PROMOTED(m) == EMPTY) {
            score = -negamax(pos, -alpha - 1, -alpha, depth - 2, sc);
            if (score > alpha) {
                score = -negamax(pos, -beta, -alpha, depth - 1, sc);
            }
        } else {
            score = -negamax(pos, -beta, -alpha, depth - 1, sc);
        }

        unmake_move(pos, m);

        if (sc->stop_search) return 0;

        if (score > alpha) {
            alpha = score;
            best_move_found = m;
            hash_flag = HASH_FLAG_EXACT;

            if (score >= beta) {
                if (!MOVE_CAPTURE(m) && pos->ply < MAX_PLY) {
                    killer_moves[1][pos->ply] = killer_moves[0][pos->ply];
                    killer_moves[0][pos->ply] = m;
                    history_moves[MOVE_PIECE(m)][MOVE_TO(m)] += depth * depth;
                }
                write_tt(pos->hash_key, depth, HASH_FLAG_BETA, beta, m, pos->ply);
                return beta;
            }
        }
    }

    if (legal_moves == 0) {
        if (in_chk) {
            return -MATE_SCORE + pos->ply; // Checkmate
        } else {
            return 0; // Stalemate
        }
    }

    write_tt(pos->hash_key, depth, hash_flag, alpha, best_move_found, pos->ply);
    return alpha;
}

static int get_pv_line(Position *pos, Move *pv_array, int max_depth) {
    int count = 0;
    Position temp_pos = *pos;
    Move best_move = 0;

    while (count < max_depth) {
        read_tt(temp_pos.hash_key, 0, -30000, 30000, &best_move, count);
        if (best_move == 0) break;

        MoveList list;
        generate_moves(&temp_pos, &list);
        int legal = 0;
        for (int i = 0; i < list.count; i++) {
            if (list.moves[i] == best_move) {
                legal = 1;
                break;
            }
        }
        if (!legal) break;

        pv_array[count++] = best_move;
        if (!make_move(&temp_pos, best_move)) break;
    }
    return count;
}

Move search_position(Position *pos, SearchControl *sc) {
    reset_search_heuristics();
    sc->nodes = 0;
    sc->stop_search = 0;
    sc->start_time = get_time_ms();
    if (sc->time_limit_ms != -1) {
        sc->stop_time = sc->start_time + sc->time_limit_ms;
    }

    Move best_move = 0;
    Move pv_line[MAX_PLY];

    for (int depth = 1; depth <= sc->depth_limit; depth++) {
        int score = negamax(pos, -30000, 30000, depth, sc);
        if (sc->stop_search) break;

        uint64_t elapsed = get_time_ms() - sc->start_time;
        if (elapsed == 0) elapsed = 1;
        uint64_t nps = (sc->nodes * 1000) / elapsed;

        int pv_count = get_pv_line(pos, pv_line, depth);
        if (pv_count > 0) {
            best_move = pv_line[0];
        }

        // Print UCI info string
        printf("info depth %d score ", depth);
        if (score > IS_MATE) {
            printf("mate %d ", (MATE_SCORE - score + 1) / 2);
        } else if (score < -IS_MATE) {
            printf("mate %d ", (-MATE_SCORE - score) / 2);
        } else {
            printf("cp %d ", score);
        }
        printf("nodes %llu nps %llu time %llu pv", (unsigned long long)sc->nodes, (unsigned long long)nps, (unsigned long long)elapsed);

        for (int i = 0; i < pv_count; i++) {
            printf(" ");
            print_move(pv_line[i]);
        }
        printf("\n");
        fflush(stdout);

        if (score > IS_MATE || score < -IS_MATE) break;
    }

    return best_move;
}
