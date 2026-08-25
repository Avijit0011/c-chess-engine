#include "move.h"
#include "movegen.h"

void add_move(MoveList *list, Move move) {
    if (list->count < MAX_MOVES) {
        list->moves[list->count++] = move;
    }
}

void sprintf_move(Move move, char *str) {
    if (move == 0) {
        strcpy(str, "(none)");
        return;
    }

    int from = MOVE_FROM(move);
    int to = MOVE_TO(move);
    int promoted = MOVE_PROMOTED(move);

    int idx = 0;
    str[idx++] = square_to_file(from);
    str[idx++] = square_to_rank(from);
    str[idx++] = square_to_file(to);
    str[idx++] = square_to_rank(to);

    if (promoted != EMPTY) {
        switch (promoted) {
            case Q: case q: str[idx++] = 'q'; break;
            case R: case r: str[idx++] = 'r'; break;
            case B: case b: str[idx++] = 'b'; break;
            case N: case n: str[idx++] = 'n'; break;
        }
    }
    str[idx] = '\0';
}

void print_move(Move move) {
    char move_str[10];
    sprintf_move(move, move_str);
    printf("%s", move_str);
}

Move parse_move_string(const char *move_str, const Position *pos) {
    if (!move_str || strlen(move_str) < 4) return 0;

    int from = (move_str[0] - 'a') + (move_str[1] - '1') * 8;
    int to = (move_str[2] - 'a') + (move_str[3] - '1') * 8;

    if (from < 0 || from > 63 || to < 0 || to > 63) return 0;

    MoveList list;
    generate_moves(pos, &list);

    for (int i = 0; i < list.count; i++) {
        Move m = list.moves[i];
        if ((int)MOVE_FROM(m) == from && (int)MOVE_TO(m) == to) {
            int promoted = MOVE_PROMOTED(m);
            if (promoted != EMPTY) {
                char prom_char = move_str[4];
                if ((prom_char == 'q' || prom_char == 'Q') && (promoted == Q || promoted == q)) return m;
                if ((prom_char == 'r' || prom_char == 'R') && (promoted == R || promoted == r)) return m;
                if ((prom_char == 'b' || prom_char == 'B') && (promoted == B || promoted == b)) return m;
                if ((prom_char == 'n' || prom_char == 'N') && (promoted == N || promoted == n)) return m;
                continue;
            }
            return m;
        }
    }

    return 0;
}
