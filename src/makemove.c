#include "makemove.h"

const int castling_rights_mask[64] = {
    13, 15, 15, 15, 12, 15, 15, 14, // Rank 1 (A1=13, E1=12, H1=14)
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
     7, 15, 15, 15,  3, 15, 15, 11  // Rank 8 (A8=7, E8=3, H8=11)
};

int make_move(Position *pos, Move move) {
    if (pos->his_ply >= 2000) return 0;

    int from = MOVE_FROM(move);
    int to = MOVE_TO(move);
    int piece = MOVE_PIECE(move);
    int promoted = MOVE_PROMOTED(move);
    int capture = MOVE_CAPTURE(move);
    int double_push = MOVE_DOUBLE_PUSH(move);
    int enpassant = MOVE_ENPASSANT(move);
    int castling = MOVE_CASTLE(move);

    int side = pos->side;
    int enemy = 1 - side;

    // Save state in history stack
    pos->history[pos->his_ply].move = move;
    pos->history[pos->his_ply].castling = pos->castling;
    pos->history[pos->his_ply].enpassant = pos->enpassant;
    pos->history[pos->his_ply].fifty_move = pos->fifty_move;
    pos->history[pos->his_ply].hash_key = pos->hash_key;

    if (enpassant) {
        pos->history[pos->his_ply].captured_piece = (side == WHITE) ? p : P;
    } else {
        pos->history[pos->his_ply].captured_piece = pos->board[to];
    }

    // Reset enpassant in hash key if present
    if (pos->enpassant != NO_SQ) {
        pos->hash_key ^= enpassant_keys[pos->enpassant];
    }
    pos->enpassant = NO_SQ;

    // Fifty-move clock
    if (piece == P || piece == p || capture) {
        pos->fifty_move = 0;
    } else {
        pos->fifty_move++;
    }

    // Handle normal capture
    if (capture && !enpassant) {
        int captured_piece = pos->board[to];
        if (captured_piece != EMPTY) {
            POP_BIT(pos->bitboards[captured_piece - 1], to);
            POP_BIT(pos->occupancy[enemy], to);
            POP_BIT(pos->occupancy[BOTH], to);
            pos->hash_key ^= piece_keys[captured_piece - 1][to];
            pos->board[to] = EMPTY;
        }
    }

    // Move source piece
    POP_BIT(pos->bitboards[piece - 1], from);
    POP_BIT(pos->occupancy[side], from);
    POP_BIT(pos->occupancy[BOTH], from);
    pos->board[from] = EMPTY;
    pos->hash_key ^= piece_keys[piece - 1][from];

    // Place moved/promoted piece
    if (promoted != EMPTY) {
        SET_BIT(pos->bitboards[promoted - 1], to);
        SET_BIT(pos->occupancy[side], to);
        SET_BIT(pos->occupancy[BOTH], to);
        pos->board[to] = promoted;
        pos->hash_key ^= piece_keys[promoted - 1][to];
    } else {
        SET_BIT(pos->bitboards[piece - 1], to);
        SET_BIT(pos->occupancy[side], to);
        SET_BIT(pos->occupancy[BOTH], to);
        pos->board[to] = piece;
        pos->hash_key ^= piece_keys[piece - 1][to];
    }

    // Handle En Passant Capture
    if (enpassant) {
        int ep_captured_sq = (side == WHITE) ? (to - 8) : (to + 8);
        int ep_piece = (side == WHITE) ? p : P;
        POP_BIT(pos->bitboards[ep_piece - 1], ep_captured_sq);
        POP_BIT(pos->occupancy[enemy], ep_captured_sq);
        POP_BIT(pos->occupancy[BOTH], ep_captured_sq);
        pos->board[ep_captured_sq] = EMPTY;
        pos->hash_key ^= piece_keys[ep_piece - 1][ep_captured_sq];
    }

    // Handle Double Pawn Push
    if (double_push) {
        pos->enpassant = (side == WHITE) ? (from + 8) : (from - 8);
        pos->hash_key ^= enpassant_keys[pos->enpassant];
    }

    // Handle Castling Rook Move
    if (castling) {
        switch (to) {
            case G1: // White Kingside
                POP_BIT(pos->bitboards[R - 1], H1); POP_BIT(pos->occupancy[WHITE], H1); POP_BIT(pos->occupancy[BOTH], H1); pos->board[H1] = EMPTY; pos->hash_key ^= piece_keys[R - 1][H1];
                SET_BIT(pos->bitboards[R - 1], F1); SET_BIT(pos->occupancy[WHITE], F1); SET_BIT(pos->occupancy[BOTH], F1); pos->board[F1] = R;     pos->hash_key ^= piece_keys[R - 1][F1];
                break;
            case C1: // White Queenside
                POP_BIT(pos->bitboards[R - 1], A1); POP_BIT(pos->occupancy[WHITE], A1); POP_BIT(pos->occupancy[BOTH], A1); pos->board[A1] = EMPTY; pos->hash_key ^= piece_keys[R - 1][A1];
                SET_BIT(pos->bitboards[R - 1], D1); SET_BIT(pos->occupancy[WHITE], D1); SET_BIT(pos->occupancy[BOTH], D1); pos->board[D1] = R;     pos->hash_key ^= piece_keys[R - 1][D1];
                break;
            case G8: // Black Kingside
                POP_BIT(pos->bitboards[r - 1], H8); POP_BIT(pos->occupancy[BLACK], H8); POP_BIT(pos->occupancy[BOTH], H8); pos->board[H8] = EMPTY; pos->hash_key ^= piece_keys[r - 1][H8];
                SET_BIT(pos->bitboards[r - 1], F8); SET_BIT(pos->occupancy[BLACK], F8); SET_BIT(pos->occupancy[BOTH], F8); pos->board[F8] = r;     pos->hash_key ^= piece_keys[r - 1][F8];
                break;
            case C8: // Black Queenside
                POP_BIT(pos->bitboards[r - 1], A8); POP_BIT(pos->occupancy[BLACK], A8); POP_BIT(pos->occupancy[BOTH], A8); pos->board[A8] = EMPTY; pos->hash_key ^= piece_keys[r - 1][A8];
                SET_BIT(pos->bitboards[r - 1], D8); SET_BIT(pos->occupancy[BLACK], D8); SET_BIT(pos->occupancy[BOTH], D8); pos->board[D8] = r;     pos->hash_key ^= piece_keys[r - 1][D8];
                break;
        }
    }

    // Update castling rights
    pos->hash_key ^= castling_keys[pos->castling];
    pos->castling &= castling_rights_mask[from];
    pos->castling &= castling_rights_mask[to];
    pos->hash_key ^= castling_keys[pos->castling];

    // Toggle side to move
    pos->side = 1 - pos->side;
    pos->hash_key ^= side_key;

    pos->ply++;
    pos->his_ply++;

    // Check move legality (king must NOT be in check after move)
    if (in_check(pos, side)) {
        unmake_move(pos, move);
        return 0;
    }

    return 1;
}

void unmake_move(Position *pos, Move move) {
    pos->ply--;
    pos->his_ply--;

    pos->side = 1 - pos->side;

    int from = MOVE_FROM(move);
    int to = MOVE_TO(move);
    int piece = MOVE_PIECE(move);
    int promoted = MOVE_PROMOTED(move);
    int capture = MOVE_CAPTURE(move);
    int enpassant = MOVE_ENPASSANT(move);
    int castling = MOVE_CASTLE(move);

    int side = pos->side;
    int enemy = 1 - side;

    int captured_pc = pos->history[pos->his_ply].captured_piece;

    // Remove piece from target square
    if (promoted != EMPTY) {
        POP_BIT(pos->bitboards[promoted - 1], to);
        POP_BIT(pos->occupancy[side], to);
        POP_BIT(pos->occupancy[BOTH], to);
        pos->board[to] = EMPTY;
    } else {
        POP_BIT(pos->bitboards[piece - 1], to);
        POP_BIT(pos->occupancy[side], to);
        POP_BIT(pos->occupancy[BOTH], to);
        pos->board[to] = EMPTY;
    }

    // Put moved piece back to source square
    SET_BIT(pos->bitboards[piece - 1], from);
    SET_BIT(pos->occupancy[side], from);
    SET_BIT(pos->occupancy[BOTH], from);
    pos->board[from] = piece;

    // Restore normal capture
    if (capture && !enpassant) {
        if (captured_pc != EMPTY) {
            SET_BIT(pos->bitboards[captured_pc - 1], to);
            SET_BIT(pos->occupancy[enemy], to);
            SET_BIT(pos->occupancy[BOTH], to);
            pos->board[to] = captured_pc;
        }
    }

    // Restore En Passant capture
    if (enpassant) {
        int ep_captured_sq = (side == WHITE) ? (to - 8) : (to + 8);
        int ep_piece = (side == WHITE) ? p : P;
        SET_BIT(pos->bitboards[ep_piece - 1], ep_captured_sq);
        SET_BIT(pos->occupancy[enemy], ep_captured_sq);
        SET_BIT(pos->occupancy[BOTH], ep_captured_sq);
        pos->board[ep_captured_sq] = ep_piece;
    }

    // Restore Castling Rook
    if (castling) {
        switch (to) {
            case G1:
                POP_BIT(pos->bitboards[R - 1], F1); POP_BIT(pos->occupancy[WHITE], F1); POP_BIT(pos->occupancy[BOTH], F1); pos->board[F1] = EMPTY;
                SET_BIT(pos->bitboards[R - 1], H1); SET_BIT(pos->occupancy[WHITE], H1); SET_BIT(pos->occupancy[BOTH], H1); pos->board[H1] = R;
                break;
            case C1:
                POP_BIT(pos->bitboards[R - 1], D1); POP_BIT(pos->occupancy[WHITE], D1); POP_BIT(pos->occupancy[BOTH], D1); pos->board[D1] = EMPTY;
                SET_BIT(pos->bitboards[R - 1], A1); SET_BIT(pos->occupancy[WHITE], A1); SET_BIT(pos->occupancy[BOTH], A1); pos->board[A1] = R;
                break;
            case G8:
                POP_BIT(pos->bitboards[r - 1], F8); POP_BIT(pos->occupancy[BLACK], F8); POP_BIT(pos->occupancy[BOTH], F8); pos->board[F8] = EMPTY;
                SET_BIT(pos->bitboards[r - 1], H8); SET_BIT(pos->occupancy[BLACK], H8); SET_BIT(pos->occupancy[BOTH], H8); pos->board[H8] = r;
                break;
            case C8:
                POP_BIT(pos->bitboards[r - 1], D8); POP_BIT(pos->occupancy[BLACK], D8); POP_BIT(pos->occupancy[BOTH], D8); pos->board[D8] = EMPTY;
                SET_BIT(pos->bitboards[r - 1], A8); SET_BIT(pos->occupancy[BLACK], A8); SET_BIT(pos->occupancy[BOTH], A8); pos->board[A8] = r;
                break;
        }
    }

    // Restore position variables from history
    pos->castling = pos->history[pos->his_ply].castling;
    pos->enpassant = pos->history[pos->his_ply].enpassant;
    pos->fifty_move = pos->history[pos->his_ply].fifty_move;
    pos->hash_key = pos->history[pos->his_ply].hash_key;
}

void make_null_move(Position *pos) {
    pos->history[pos->his_ply].move = 0;
    pos->history[pos->his_ply].castling = pos->castling;
    pos->history[pos->his_ply].enpassant = pos->enpassant;
    pos->history[pos->his_ply].fifty_move = pos->fifty_move;
    pos->history[pos->his_ply].hash_key = pos->hash_key;
    pos->history[pos->his_ply].captured_piece = EMPTY;

    if (pos->enpassant != NO_SQ) {
        pos->hash_key ^= enpassant_keys[pos->enpassant];
        pos->enpassant = NO_SQ;
    }

    pos->side = 1 - pos->side;
    pos->hash_key ^= side_key;

    pos->ply++;
    pos->his_ply++;
}

void unmake_null_move(Position *pos) {
    pos->ply--;
    pos->his_ply--;

    pos->side = 1 - pos->side;

    pos->castling = pos->history[pos->his_ply].castling;
    pos->enpassant = pos->history[pos->his_ply].enpassant;
    pos->fifty_move = pos->history[pos->his_ply].fifty_move;
    pos->hash_key = pos->history[pos->his_ply].hash_key;
}
