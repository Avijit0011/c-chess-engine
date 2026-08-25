#include "position.h"

uint64_t piece_keys[12][64];
uint64_t enpassant_keys[64];
uint64_t castling_keys[16];
uint64_t side_key;

const char piece_ascii[13] = ".PNBRQKpnbrqk";
const char *piece_unicode[13] = { ".", "P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k" };

// 64-bit Pseudo-random number generator (xorshift64)
static uint64_t random_uint64_state = 1804289383ULL;

static uint64_t get_random_uint64(void) {
    uint64_t x = random_uint64_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return random_uint64_state = x;
}

void init_zobrist(void) {
    random_uint64_state = 1804289383ULL;
    for (int p = 0; p < 12; p++) {
        for (int sq = 0; sq < 64; sq++) {
            piece_keys[p][sq] = get_random_uint64();
        }
    }
    for (int sq = 0; sq < 64; sq++) {
        enpassant_keys[sq] = get_random_uint64();
    }
    for (int i = 0; i < 16; i++) {
        castling_keys[i] = get_random_uint64();
    }
    side_key = get_random_uint64();
}

uint64_t generate_hash_key(const Position *pos) {
    uint64_t final_key = 0ULL;

    for (int sq = 0; sq < 64; sq++) {
        int piece = pos->board[sq];
        if (piece != EMPTY) {
            final_key ^= piece_keys[piece - 1][sq];
        }
    }

    if (pos->side == WHITE) {
        final_key ^= side_key;
    }

    if (pos->enpassant != NO_SQ) {
        final_key ^= enpassant_keys[pos->enpassant];
    }

    final_key ^= castling_keys[pos->castling];

    return final_key;
}

void reset_position(Position *pos) {
    memset(pos, 0, sizeof(Position));
    for (int i = 0; i < 64; i++) {
        pos->board[i] = EMPTY;
    }
    pos->enpassant = NO_SQ;
    pos->side = WHITE;
    pos->castling = 0;
    pos->fifty_move = 0;
    pos->ply = 0;
    pos->his_ply = 0;
}

int char_to_piece(char c) {
    switch (c) {
        case 'P': return P;
        case 'N': return N;
        case 'B': return B;
        case 'R': return R;
        case 'Q': return Q;
        case 'K': return K;
        case 'p': return p;
        case 'n': return n;
        case 'b': return b;
        case 'r': return r;
        case 'q': return q;
        case 'k': return k;
        default: return EMPTY;
    }
}

char square_to_file(int sq) {
    return 'a' + (sq % 8);
}

char square_to_rank(int sq) {
    return '1' + (sq / 8);
}

int parse_fen(Position *pos, const char *fen) {
    if (!fen) return 0;
    reset_position(pos);

    int r = 7;
    int f = 0;

    const char *ptr = fen;

    // 1. Piece placement
    while (r >= 0 && *ptr && *ptr != ' ') {
        if (*ptr == '/') {
            r--;
            f = 0;
        } else if (isdigit((unsigned char)*ptr)) {
            f += (*ptr - '0');
        } else {
            int piece = char_to_piece(*ptr);
            if (piece != EMPTY && f >= 0 && f < 8 && r >= 0 && r < 8) {
                int sq = r * 8 + f;
                pos->board[sq] = piece;
                SET_BIT(pos->bitboards[piece - 1], sq);
                f++;
            }
        }
        ptr++;
    }

    // Update occupancies
    for (int pc = P; pc <= K; pc++) {
        pos->occupancy[WHITE] |= pos->bitboards[pc - 1];
    }
    for (int pc = p; pc <= k; pc++) {
        pos->occupancy[BLACK] |= pos->bitboards[pc - 1];
    }
    pos->occupancy[BOTH] = pos->occupancy[WHITE] | pos->occupancy[BLACK];

    // 2. Side to move
    while (*ptr == ' ') ptr++;
    if (*ptr == 'w') {
        pos->side = WHITE;
    } else if (*ptr == 'b') {
        pos->side = BLACK;
    }
    if (*ptr) ptr++;

    // 3. Castling rights
    while (*ptr == ' ') ptr++;
    while (*ptr && *ptr != ' ') {
        if (*ptr == 'K') pos->castling |= WK;
        else if (*ptr == 'Q') pos->castling |= WQ;
        else if (*ptr == 'k') pos->castling |= BK;
        else if (*ptr == 'q') pos->castling |= BQ;
        ptr++;
    }

    // 4. En-passant square
    while (*ptr == ' ') ptr++;
    if (*ptr && *ptr != '-') {
        int file = ptr[0] - 'a';
        int rank = ptr[1] - '1';
        if (file >= 0 && file <= 7 && rank >= 0 && rank <= 7) {
            pos->enpassant = rank * 8 + file;
        }
        ptr += 2;
    } else if (*ptr == '-') {
        pos->enpassant = NO_SQ;
        ptr++;
    }

    // 5. Halfmove clock
    while (*ptr == ' ') ptr++;
    if (*ptr && isdigit((unsigned char)*ptr)) {
        pos->fifty_move = atoi(ptr);
        while (*ptr && isdigit((unsigned char)*ptr)) ptr++;
    }

    // 6. Fullmove counter (stored if needed)

    // Compute hash key
    pos->hash_key = generate_hash_key(pos);

    return 1;
}

void generate_fen(const Position *pos, char *fen) {
    int idx = 0;
    for (int r = 7; r >= 0; r--) {
        int empty_count = 0;
        for (int f = 0; f < 8; f++) {
            int sq = r * 8 + f;
            int piece = pos->board[sq];
            if (piece == EMPTY) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    fen[idx++] = '0' + empty_count;
                    empty_count = 0;
                }
                fen[idx++] = piece_ascii[piece];
            }
        }
        if (empty_count > 0) {
            fen[idx++] = '0' + empty_count;
        }
        if (r > 0) fen[idx++] = '/';
    }

    fen[idx++] = ' ';
    fen[idx++] = (pos->side == WHITE) ? 'w' : 'b';
    fen[idx++] = ' ';

    if (pos->castling == 0) {
        fen[idx++] = '-';
    } else {
        if (pos->castling & WK) fen[idx++] = 'K';
        if (pos->castling & WQ) fen[idx++] = 'Q';
        if (pos->castling & BK) fen[idx++] = 'k';
        if (pos->castling & BQ) fen[idx++] = 'q';
    }

    fen[idx++] = ' ';
    if (pos->enpassant == NO_SQ) {
        fen[idx++] = '-';
    } else {
        fen[idx++] = square_to_file(pos->enpassant);
        fen[idx++] = square_to_rank(pos->enpassant);
    }

    sprintf(&fen[idx], " %d %d", pos->fifty_move, pos->his_ply / 2 + 1);
}

void print_board(const Position *pos) {
    printf("\n");
    for (int r = 7; r >= 0; r--) {
        printf(" %d  ", r + 1);
        for (int f = 0; f < 8; f++) {
            int sq = r * 8 + f;
            int piece = pos->board[sq];
            printf(" %c", piece_ascii[piece]);
        }
        printf("\n");
    }
    printf("\n    a b c d e f g h\n\n");
    printf(" Side to move: %s\n", (pos->side == WHITE) ? "White" : "Black");
    printf(" Castling:    %c%c%c%c\n",
        (pos->castling & WK) ? 'K' : '-',
        (pos->castling & WQ) ? 'Q' : '-',
        (pos->castling & BK) ? 'k' : '-',
        (pos->castling & BQ) ? 'q' : '-');
    if (pos->enpassant != NO_SQ) {
        printf(" En Passant:   %c%c\n", square_to_file(pos->enpassant), square_to_rank(pos->enpassant));
    } else {
        printf(" En Passant:   -\n");
    }
    printf(" Hash Key:     0x%llxULL\n", (unsigned long long)pos->hash_key);
    char fen_buf[128];
    generate_fen(pos, fen_buf);
    printf(" FEN:          %s\n\n", fen_buf);
}
