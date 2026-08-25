#include "tt.h"

TranspositionTable TT = { NULL, 0 };

void init_tt(int mb_size) {
    if (mb_size < 1) mb_size = 1;
    if (mb_size > 1024) mb_size = 1024;

    free_tt();

    size_t bytes = (size_t)mb_size * 1024 * 1024;
    TT.count = (int)(bytes / sizeof(TTEntry));
    TT.entries = (TTEntry *)calloc(TT.count, sizeof(TTEntry));
}

void free_tt(void) {
    if (TT.entries) {
        free(TT.entries);
        TT.entries = NULL;
    }
    TT.count = 0;
}

void clear_tt(void) {
    if (TT.entries && TT.count > 0) {
        memset(TT.entries, 0, TT.count * sizeof(TTEntry));
    }
}

int read_tt(uint64_t hash_key, int depth, int alpha, int beta, Move *best_move, int ply) {
    if (!TT.entries || TT.count == 0) return 0;

    int idx = (int)(hash_key % TT.count);
    TTEntry *entry = &TT.entries[idx];

    if (entry->hash_key == hash_key) {
        if (best_move) *best_move = entry->best_move;

        if (entry->depth >= depth) {
            int score = entry->score;
            if (score > IS_MATE) score -= ply;
            if (score < -IS_MATE) score += ply;

            if (entry->flag == HASH_FLAG_EXACT) return score;
            if (entry->flag == HASH_FLAG_ALPHA && score <= alpha) return alpha;
            if (entry->flag == HASH_FLAG_BETA && score >= beta) return beta;
        }
    }

    return 100000; // Return invalid score indicator (no TT hit)
}

void write_tt(uint64_t hash_key, int depth, int flag, int score, Move best_move, int ply) {
    if (!TT.entries || TT.count == 0) return;

    int idx = (int)(hash_key % TT.count);
    TTEntry *entry = &TT.entries[idx];

    // Always replace or replace if deeper/same position
    if (entry->hash_key == 0 || entry->depth <= depth || entry->hash_key != hash_key) {
        if (score > IS_MATE) score += ply;
        if (score < -IS_MATE) score -= ply;

        entry->hash_key = hash_key;
        entry->depth = depth;
        entry->flag = flag;
        entry->score = score;
        if (best_move != 0) {
            entry->best_move = best_move;
        }
    }
}
