#ifndef TT_H
#define TT_H

#include "defs.h"

#define HASH_FLAG_EXACT 0
#define HASH_FLAG_ALPHA 1
#define HASH_FLAG_BETA  2

#define IS_MATE 19000
#define MATE_SCORE 20000

typedef struct {
    uint64_t hash_key;
    int depth;
    int flag;
    int score;
    Move best_move;
} TTEntry;

typedef struct {
    TTEntry *entries;
    int count;
} TranspositionTable;

extern TranspositionTable TT;

void init_tt(int mb_size);
void free_tt(void);
void clear_tt(void);

int read_tt(uint64_t hash_key, int depth, int alpha, int beta, Move *best_move, int ply);
void write_tt(uint64_t hash_key, int depth, int flag, int score, Move best_move, int ply);

#endif // TT_H
