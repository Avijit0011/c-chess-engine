#include "movegen.h"
#include "move.h"

int is_square_attacked(const Position *pos, int square, int attacker_side) {
    // 1. Pawn attacks
    if (attacker_side == WHITE) {
        if (pawn_attacks[BLACK][square] & pos->bitboards[P - 1]) return 1;
    } else {
        if (pawn_attacks[WHITE][square] & pos->bitboards[p - 1]) return 1;
    }

    // 2. Knight attacks
    int knight_piece = (attacker_side == WHITE) ? N : n;
    if (knight_attacks[square] & pos->bitboards[knight_piece - 1]) return 1;

    // 3. Bishop & Queen attacks (diagonals)
    int bishop_piece = (attacker_side == WHITE) ? B : b;
    int queen_piece  = (attacker_side == WHITE) ? Q : q;
    Bitboard bishop_queen = pos->bitboards[bishop_piece - 1] | pos->bitboards[queen_piece - 1];
    if (get_bishop_attacks(square, pos->occupancy[BOTH]) & bishop_queen) return 1;

    // 4. Rook & Queen attacks (orthogonals)
    int rook_piece  = (attacker_side == WHITE) ? R : r;
    Bitboard rook_queen = pos->bitboards[rook_piece - 1] | pos->bitboards[queen_piece - 1];
    if (get_rook_attacks(square, pos->occupancy[BOTH]) & rook_queen) return 1;

    // 5. King attacks
    int king_piece = (attacker_side == WHITE) ? K : k;
    if (king_attacks[square] & pos->bitboards[king_piece - 1]) return 1;

    return 0;
}

int in_check(const Position *pos, int side) {
    int king_piece = (side == WHITE) ? K : k;
    Bitboard king_bb = pos->bitboards[king_piece - 1];
    if (!king_bb) return 0;
    int king_sq = get_lsb(king_bb);
    return is_square_attacked(pos, king_sq, 1 - side);
}

void generate_moves(const Position *pos, MoveList *list) {
    list->count = 0;

    int side = pos->side;
    int enemy = 1 - side;

    if (side == WHITE) {
        // --- WHITE PAWNS ---
        Bitboard pawns = pos->bitboards[P - 1];
        while (pawns) {
            int sq = get_lsb(pawns);
            int r = sq / 8;

            // Single Push
            int to = sq + 8;
            if (to <= 63 && pos->board[to] == EMPTY) {
                if (r == RANK_7) {
                    add_move(list, ENCODE_MOVE(sq, to, P, Q, 0, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, to, P, R, 0, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, to, P, B, 0, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, to, P, N, 0, 0, 0, 0));
                } else {
                    add_move(list, ENCODE_MOVE(sq, to, P, EMPTY, 0, 0, 0, 0));
                    // Double Push
                    int double_to = sq + 16;
                    if (r == RANK_2 && pos->board[double_to] == EMPTY) {
                        add_move(list, ENCODE_MOVE(sq, double_to, P, EMPTY, 0, 1, 0, 0));
                    }
                }
            }

            // Captures
            Bitboard attacks = pawn_attacks[WHITE][sq] & pos->occupancy[BLACK];
            while (attacks) {
                int cap_to = get_lsb(attacks);
                if (r == RANK_7) {
                    add_move(list, ENCODE_MOVE(sq, cap_to, P, Q, 1, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, cap_to, P, R, 1, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, cap_to, P, B, 1, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, cap_to, P, N, 1, 0, 0, 0));
                } else {
                    add_move(list, ENCODE_MOVE(sq, cap_to, P, EMPTY, 1, 0, 0, 0));
                }
                POP_BIT(attacks, cap_to);
            }

            // En Passant
            if (pos->enpassant != NO_SQ) {
                if (GET_BIT(pawn_attacks[WHITE][sq], pos->enpassant)) {
                    add_move(list, ENCODE_MOVE(sq, pos->enpassant, P, EMPTY, 1, 0, 1, 0));
                }
            }

            POP_BIT(pawns, sq);
        }

        // --- WHITE CASTLING ---
        if (pos->castling & WK) {
            if (pos->board[F1] == EMPTY && pos->board[G1] == EMPTY) {
                if (!is_square_attacked(pos, E1, BLACK) && !is_square_attacked(pos, F1, BLACK)) {
                    add_move(list, ENCODE_MOVE(E1, G1, K, EMPTY, 0, 0, 0, 1));
                }
            }
        }
        if (pos->castling & WQ) {
            if (pos->board[D1] == EMPTY && pos->board[C1] == EMPTY && pos->board[B1] == EMPTY) {
                if (!is_square_attacked(pos, E1, BLACK) && !is_square_attacked(pos, D1, BLACK)) {
                    add_move(list, ENCODE_MOVE(E1, C1, K, EMPTY, 0, 0, 0, 1));
                }
            }
        }
    } else {
        // --- BLACK PAWNS ---
        Bitboard pawns = pos->bitboards[p - 1];
        while (pawns) {
            int sq = get_lsb(pawns);
            int r = sq / 8;

            // Single Push
            int to = sq - 8;
            if (to >= 0 && pos->board[to] == EMPTY) {
                if (r == RANK_2) {
                    add_move(list, ENCODE_MOVE(sq, to, p, q, 0, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, to, p, r, 0, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, to, p, b, 0, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, to, p, n, 0, 0, 0, 0));
                } else {
                    add_move(list, ENCODE_MOVE(sq, to, p, EMPTY, 0, 0, 0, 0));
                    // Double Push
                    int double_to = sq - 16;
                    if (r == RANK_7 && pos->board[double_to] == EMPTY) {
                        add_move(list, ENCODE_MOVE(sq, double_to, p, EMPTY, 0, 1, 0, 0));
                    }
                }
            }

            // Captures
            Bitboard attacks = pawn_attacks[BLACK][sq] & pos->occupancy[WHITE];
            while (attacks) {
                int cap_to = get_lsb(attacks);
                if (r == RANK_2) {
                    add_move(list, ENCODE_MOVE(sq, cap_to, p, q, 1, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, cap_to, p, r, 1, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, cap_to, p, b, 1, 0, 0, 0));
                    add_move(list, ENCODE_MOVE(sq, cap_to, p, n, 1, 0, 0, 0));
                } else {
                    add_move(list, ENCODE_MOVE(sq, cap_to, p, EMPTY, 1, 0, 0, 0));
                }
                POP_BIT(attacks, cap_to);
            }

            // En Passant
            if (pos->enpassant != NO_SQ) {
                if (GET_BIT(pawn_attacks[BLACK][sq], pos->enpassant)) {
                    add_move(list, ENCODE_MOVE(sq, pos->enpassant, p, EMPTY, 1, 0, 1, 0));
                }
            }

            POP_BIT(pawns, sq);
        }

        // --- BLACK CASTLING ---
        if (pos->castling & BK) {
            if (pos->board[F8] == EMPTY && pos->board[G8] == EMPTY) {
                if (!is_square_attacked(pos, E8, WHITE) && !is_square_attacked(pos, F8, WHITE)) {
                    add_move(list, ENCODE_MOVE(E8, G8, k, EMPTY, 0, 0, 0, 1));
                }
            }
        }
        if (pos->castling & BQ) {
            if (pos->board[D8] == EMPTY && pos->board[C8] == EMPTY && pos->board[B8] == EMPTY) {
                if (!is_square_attacked(pos, E8, WHITE) && !is_square_attacked(pos, D8, WHITE)) {
                    add_move(list, ENCODE_MOVE(E8, C8, k, EMPTY, 0, 0, 0, 1));
                }
            }
        }
    }

    // --- KNIGHTS ---
    int knight_pc = (side == WHITE) ? N : n;
    Bitboard knights = pos->bitboards[knight_pc - 1];
    while (knights) {
        int sq = get_lsb(knights);
        Bitboard attacks = knight_attacks[sq] & ~pos->occupancy[side];
        while (attacks) {
            int to = get_lsb(attacks);
            int capture = GET_BIT(pos->occupancy[enemy], to);
            add_move(list, ENCODE_MOVE(sq, to, knight_pc, EMPTY, capture, 0, 0, 0));
            POP_BIT(attacks, to);
        }
        POP_BIT(knights, sq);
    }

    // --- BISHOPS ---
    int bishop_pc = (side == WHITE) ? B : b;
    Bitboard bishops = pos->bitboards[bishop_pc - 1];
    while (bishops) {
        int sq = get_lsb(bishops);
        Bitboard attacks = get_bishop_attacks(sq, pos->occupancy[BOTH]) & ~pos->occupancy[side];
        while (attacks) {
            int to = get_lsb(attacks);
            int capture = GET_BIT(pos->occupancy[enemy], to);
            add_move(list, ENCODE_MOVE(sq, to, bishop_pc, EMPTY, capture, 0, 0, 0));
            POP_BIT(attacks, to);
        }
        POP_BIT(bishops, sq);
    }

    // --- ROOKS ---
    int rook_pc = (side == WHITE) ? R : r;
    Bitboard rooks = pos->bitboards[rook_pc - 1];
    while (rooks) {
        int sq = get_lsb(rooks);
        Bitboard attacks = get_rook_attacks(sq, pos->occupancy[BOTH]) & ~pos->occupancy[side];
        while (attacks) {
            int to = get_lsb(attacks);
            int capture = GET_BIT(pos->occupancy[enemy], to);
            add_move(list, ENCODE_MOVE(sq, to, rook_pc, EMPTY, capture, 0, 0, 0));
            POP_BIT(attacks, to);
        }
        POP_BIT(rooks, sq);
    }

    // --- QUEENS ---
    int queen_pc = (side == WHITE) ? Q : q;
    Bitboard queens = pos->bitboards[queen_pc - 1];
    while (queens) {
        int sq = get_lsb(queens);
        Bitboard attacks = get_queen_attacks(sq, pos->occupancy[BOTH]) & ~pos->occupancy[side];
        while (attacks) {
            int to = get_lsb(attacks);
            int capture = GET_BIT(pos->occupancy[enemy], to);
            add_move(list, ENCODE_MOVE(sq, to, queen_pc, EMPTY, capture, 0, 0, 0));
            POP_BIT(attacks, to);
        }
        POP_BIT(queens, sq);
    }

    // --- KING ---
    int king_pc = (side == WHITE) ? K : k;
    Bitboard king_bb = pos->bitboards[king_pc - 1];
    if (king_bb) {
        int sq = get_lsb(king_bb);
        Bitboard attacks = king_attacks[sq] & ~pos->occupancy[side];
        while (attacks) {
            int to = get_lsb(attacks);
            int capture = GET_BIT(pos->occupancy[enemy], to);
            add_move(list, ENCODE_MOVE(sq, to, king_pc, EMPTY, capture, 0, 0, 0));
            POP_BIT(attacks, to);
        }
    }
}

void generate_captures(const Position *pos, MoveList *list) {
    MoveList full_list;
    generate_moves(pos, &full_list);
    list->count = 0;
    for (int i = 0; i < full_list.count; i++) {
        Move m = full_list.moves[i];
        if (MOVE_CAPTURE(m) || MOVE_PROMOTED(m) != EMPTY) {
            add_move(list, m);
        }
    }
}
