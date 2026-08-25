#include "bitboard.h"

Bitboard pawn_attacks[2][64];
Bitboard knight_attacks[64];
Bitboard king_attacks[64];

int count_bits(Bitboard bb) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(bb);
#else
    int count = 0;
    while (bb) {
        count++;
        bb &= bb - 1;
    }
    return count;
#endif
}

int get_lsb(Bitboard bb) {
    if (bb == 0ULL) return -1;
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(bb);
#else
    return count_bits((bb & -bb) - 1);
#endif
}


// Explicit rank/file Pawn attack helper
static Bitboard compute_pawn_attacks(int side, int square) {
    Bitboard attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;

    if (side == WHITE) {
        if (r + 1 <= 7) {
            if (f - 1 >= 0) SET_BIT(attacks, (r + 1) * 8 + (f - 1)); // Up-Left
            if (f + 1 <= 7) SET_BIT(attacks, (r + 1) * 8 + (f + 1)); // Up-Right
        }
    } else {
        if (r - 1 >= 0) {
            if (f - 1 >= 0) SET_BIT(attacks, (r - 1) * 8 + (f - 1)); // Down-Left
            if (f + 1 <= 7) SET_BIT(attacks, (r - 1) * 8 + (f + 1)); // Down-Right
        }
    }
    return attacks;
}

static Bitboard compute_knight_attacks(int square) {
    Bitboard attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;

    int dr[] = { 2, 2, 1, 1, -1, -1, -2, -2 };
    int df[] = { 1, -1, 2, -2, 2, -2, 1, -1 };

    for (int i = 0; i < 8; i++) {
        int tr = r + dr[i];
        int tf = f + df[i];
        if (tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7) {
            SET_BIT(attacks, tr * 8 + tf);
        }
    }
    return attacks;
}

static Bitboard compute_king_attacks(int square) {
    Bitboard attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;

    int dr[] = { 1, 1, 1, 0, 0, -1, -1, -1 };
    int df[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

    for (int i = 0; i < 8; i++) {
        int tr = r + dr[i];
        int tf = f + df[i];
        if (tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7) {
            SET_BIT(attacks, tr * 8 + tf);
        }
    }
    return attacks;
}

Bitboard get_bishop_attacks(int square, Bitboard occupancy) {
    Bitboard attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;

    // NE
    for (int tr = r + 1, tf = f + 1; tr <= 7 && tf <= 7; tr++, tf++) {
        int sq = tr * 8 + tf;
        SET_BIT(attacks, sq);
        if (GET_BIT(occupancy, sq)) break;
    }
    // NW
    for (int tr = r + 1, tf = f - 1; tr <= 7 && tf >= 0; tr++, tf--) {
        int sq = tr * 8 + tf;
        SET_BIT(attacks, sq);
        if (GET_BIT(occupancy, sq)) break;
    }
    // SE
    for (int tr = r - 1, tf = f + 1; tr >= 0 && tf <= 7; tr--, tf++) {
        int sq = tr * 8 + tf;
        SET_BIT(attacks, sq);
        if (GET_BIT(occupancy, sq)) break;
    }
    // SW
    for (int tr = r - 1, tf = f - 1; tr >= 0 && tf >= 0; tr--, tf--) {
        int sq = tr * 8 + tf;
        SET_BIT(attacks, sq);
        if (GET_BIT(occupancy, sq)) break;
    }
    return attacks;
}

Bitboard get_rook_attacks(int square, Bitboard occupancy) {
    Bitboard attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;

    // North
    for (int tr = r + 1; tr <= 7; tr++) {
        int sq = tr * 8 + f;
        SET_BIT(attacks, sq);
        if (GET_BIT(occupancy, sq)) break;
    }
    // South
    for (int tr = r - 1; tr >= 0; tr--) {
        int sq = tr * 8 + f;
        SET_BIT(attacks, sq);
        if (GET_BIT(occupancy, sq)) break;
    }
    // East
    for (int tf = f + 1; tf <= 7; tf++) {
        int sq = r * 8 + tf;
        SET_BIT(attacks, sq);
        if (GET_BIT(occupancy, sq)) break;
    }
    // West
    for (int tf = f - 1; tf >= 0; tf--) {
        int sq = r * 8 + tf;
        SET_BIT(attacks, sq);
        if (GET_BIT(occupancy, sq)) break;
    }
    return attacks;
}

Bitboard get_queen_attacks(int square, Bitboard occupancy) {
    return get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
}

Bitboard mask_pawn_attacks(int side, int square) {
    return pawn_attacks[side][square];
}

Bitboard mask_knight_attacks(int square) {
    return knight_attacks[square];
}

Bitboard mask_king_attacks(int square) {
    return king_attacks[square];
}

void init_bitboards(void) {
    for (int sq = 0; sq < 64; sq++) {
        pawn_attacks[WHITE][sq] = compute_pawn_attacks(WHITE, sq);
        pawn_attacks[BLACK][sq] = compute_pawn_attacks(BLACK, sq);
        knight_attacks[sq] = compute_knight_attacks(sq);
        king_attacks[sq] = compute_king_attacks(sq);
    }
}

void print_bitboard(Bitboard bb) {
    printf("\n");
    for (int r = 7; r >= 0; r--) {
        printf(" %d  ", r + 1);
        for (int f = 0; f < 8; f++) {
            int sq = r * 8 + f;
            printf(" %c", GET_BIT(bb, sq) ? '1' : '.');
        }
        printf("\n");
    }
    printf("\n    a b c d e f g h\n\n");
    printf(" Bitboard uint64: 0x%llxULL\n\n", (unsigned long long)bb);
}
