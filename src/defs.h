#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define ENGINE_NAME "C Chess Engine"
#define ENGINE_AUTHOR "DeepMind"
#define ENGINE_VERSION "1.0"

#define MAX_MOVES 256
#define MAX_PLY 64
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef uint64_t Bitboard;

// Colors
enum { WHITE, BLACK, BOTH };

// Castling Rights Bitmasks
enum { WK = 1, WQ = 2, BK = 4, BQ = 8 };

// Piece definitions
enum { EMPTY, P, N, B, R, Q, K, p, n, b, r, q, k };

// Square definitions (A1 = 0, H8 = 63)
enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8, NO_SQ
};

// Files & Ranks
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };

// Move encoding macros (32-bit unsigned int)
typedef uint32_t Move;

#define ENCODE_MOVE(from, to, piece, promoted, capture, double_push, enpassant, castling) \
    ((from) | ((to) << 6) | ((piece) << 12) | ((promoted) << 16) | \
     ((capture) ? (1 << 20) : 0) | ((double_push) ? (1 << 21) : 0) | \
     ((enpassant) ? (1 << 22) : 0) | ((castling) ? (1 << 23) : 0))

#define MOVE_FROM(m)        ((m) & 0x3F)
#define MOVE_TO(m)          (((m) >> 6) & 0x3F)
#define MOVE_PIECE(m)       (((m) >> 12) & 0x0F)
#define MOVE_PROMOTED(m)    (((m) >> 16) & 0x0F)
#define MOVE_CAPTURE(m)     (((m) >> 20) & 0x01)
#define MOVE_DOUBLE_PUSH(m) (((m) >> 21) & 0x01)
#define MOVE_ENPASSANT(m)   (((m) >> 22) & 0x01)
#define MOVE_CASTLE(m)      (((m) >> 23) & 0x01)

typedef struct {
    Move moves[MAX_MOVES];
    int count;
} MoveList;

// Undo structure for unmaking moves
typedef struct {
    Move move;
    int castling;
    int enpassant;
    int fifty_move;
    int captured_piece;
    uint64_t hash_key;
} Undo;

// Board Position Structure
typedef struct {
    Bitboard bitboards[12]; // P, N, B, R, Q, K, p, n, b, r, q, k
    Bitboard occupancy[3];  // WHITE, BLACK, BOTH
    int board[64];          // Mailbox 8x8 lookup for square piece
    int side;               // WHITE or BLACK
    int enpassant;          // NO_SQ or square 0..63
    int castling;           // Bitmask of WK, WQ, BK, BQ
    int fifty_move;         // Halfmove clock
    int ply;                // Search ply depth
    int his_ply;            // Game history ply depth
    uint64_t hash_key;      // Zobrist position hash key
    Undo history[1024];     // History stack for undoing moves
} Position;

#endif // DEFS_H
