#include <stdio.h>
#include <stdlib.h>

struct BOARD {
    int piece[25];
    int state[25];
    int selecter_pos;
    char turn;
    int P1_point;
    int P2_point;
    struct BOARD *prev;
    struct BOARD *next;
};

#define B_NUM 0 // number
#define B_SEL 1 // selecter
#define B_NON 2 // none
#define B_OVER 9 // game over(:= B_NON * 4 + B_SEL)

// B always represents latest board state
struct BOARD *B;
#define P1_TURN 0
#define P2_TURN 1

void init_board() {
    // 0: selecter, oth: number
    int pieces[25] = {9, -9, 8, -8, 7, -7, 6, -6, 5, -5, 4, -4,
                      3, -3, 3, -3, 2, -2, 2, -2, 1, -1, 1, -1, 0};

    // 1: 選ばれた, 0: 選ばれていない
    char choice_array[25] = {0};

    // Fisher-Yates shuffle
    for (int p_pos = 0; p_pos < 25; p_pos++) {
        int choice_count = 0;

        for (int i = 0; i < 25; i++) {
            if (choice_array[i] == 1) {
                choice_count += 1;
            }
        }

        int rand_n = arc4random() % (25 - choice_count);

        int abs_count = 0;
        int abs_index;
        for (int j = 0; j < 25; j++) {
            if (choice_array[j] == 0) {
                if (abs_count == rand_n) {
                    abs_index = j;
                    break;
                }
                abs_count++;
            }
        }

        choice_array[abs_index] = 1;

        B->state[p_pos] = B_NUM;
        if (pieces[abs_index] == 0) {
            B->selecter_pos = p_pos;
            B->state[p_pos] = B_SEL;
        }
        B->piece[p_pos] = pieces[abs_index];
    }
}

void init() {
    init_board();
    B->turn = P1_TURN;
    B->P1_point = 0;
    B->P2_point = 0;
    B->prev = NULL;
    B->next = NULL;
}

void print_cell(int index) {
    char s = B->state[index];
    if (s == B_SEL) {
        printf("  X|");
    } else if (s == B_NON) {
        printf("   |");
    } else {
        printf("%3d|", B->piece[index]);
    }
}

void print_board() {
    printf("+-------------------+\n");
    for (int i = 0; i < 25; i++) {
        if (i % 5 == 0) {
            printf("|");
        }

        print_cell(i);

        if ((i + 1) % 5 == 0) {
            printf(" %d", (i / 5) + 1);
        }
        if (0 < i && i < 24 && (i + 1) % 5 == 0) {
            printf("\n|---+---+---+---+---|\n");
        }
    }
    printf("\n+-------------------+\n");
    printf("  a   b   c   d   e\n\n");
}

// for debug
void print_state() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            printf("  %d", B->state[i*5 + j]);
        }
        printf("\n");
    }
}

void print_result() {
    int p1 = B->P1_point;
    int p2 = B->P2_point;

    printf("%d - %d\n", p1, p2);
    if (p1 > p2) {
        printf("you win\n");
    } else if (p1 < p2) {
        printf("you lose\n");
    } else {
        printf("draw\n");
    }
}

int is_gameover() {
    int s_pos = B->selecter_pos;
    int s_row = s_pos / 5;
    int s_col = s_pos % 5;

    int state_sum = 0;
    if (B->turn == P1_TURN) {
        for (int i = 0; i < 5; i++) {
            state_sum += B->state[s_row * 5 + i];
        }
    } else {
        for (int i = 0; i < 5; i++) {
            state_sum += B->state[s_col + i * 5];
        }
    }

    if (state_sum == B_OVER) {
        return 1;
    }
    return 0;
}

int move2pos(char move[2]) {
    int row = move[1] - '1';
    int col = move[0] - 'a';
    return row * 5 + col;
}

void pos2move(int pos, char move[2]) {
    int row = pos / 5;
    int col = pos % 5;
    move[0] = col + 'a';
    move[1] = row + '1';
}

int is_legal_move_form(int m_pos) {
    char move[2];
    pos2move(m_pos, move);
    int row = move[1] - '1';
    int col = move[0] - 'a';
    if (0 <= row && row <= 4 && 0 <= col && col <= 4) {
        return 1;
    }
    return 0;
}

int is_legal_move(int m_pos) {
    if (!is_legal_move_form(m_pos)) {
        printf("input needs in a1,..., e5\n");
        return 0;
    }

    int s_pos = B->selecter_pos;
    int s_row = s_pos / 5;
    int s_col = s_pos % 5;

    int m_row = m_pos / 5;
    int m_col = m_pos % 5;

    if (B->turn == P1_TURN) {
        // allow only horizontal move
        if (s_pos != m_pos && s_row == m_row && B->state[m_pos] != B_NON) {
            return 1;
        }
        return 0;
    } else {
        // allow only vertical move
        if (s_pos != m_pos && s_col == m_col && B->state[m_pos] != B_NON) {
            return 1;
        }
        return 0;
    }
}

// return the valid moves array(vma).
// vma's 1st element is length of the vma except itself.
// ex. [3, 5, 6, 9] means that valid moves is 5, 6, 9(a2, b2, e2).
int *get_legal_moves() {
    int s_pos = B->selecter_pos;
    int s_row = s_pos / 5;
    int s_col = s_pos % 5;

    int *ret_arr = malloc(sizeof(int) * 5);
    int legal_moves_count = 0;

    // search in a column
    if (B->turn == P2_TURN) {
        for (int i = 0; i < 5; i++) {
            int pos = i * 5 + s_col;
            if (i == s_row || B->state[pos] == B_NON) {
                continue;
            }
            legal_moves_count++;
            ret_arr[legal_moves_count] = pos;
        }
    }

    // search in a row
    if (B->turn == P1_TURN) {
        for (int i = 0; i < 5; i++) {
            int pos = i + s_row * 5;
            if (i == s_col || B->state[pos] == B_NON) {
                continue;
            }
            legal_moves_count++;
            ret_arr[legal_moves_count] = pos;
        }
    }

    ret_arr[0] = legal_moves_count;
    return ret_arr;
}

// Sava the `B` at the time
struct BOARD *save_B() {
    struct BOARD *b = malloc(sizeof(struct BOARD));
    *b = *B;
    return b;
}

void push_pos(int pos) {
    struct BOARD *B_prev = save_B();

    B->state[pos] = B_SEL;
    B->state[B->selecter_pos] = B_NON;
    B->selecter_pos = pos;

    // P1_TURN => P2_TURN, P2_TURN => P1_TURN
    B->turn ^= 1;

    struct BOARD *B_next = save_B();
    B_next->prev = B_prev;
    B_prev->next = B_next;
    B = B_next;
}

void pop() {
    B = B->prev;
}

int get_imm_opt_move() {
    int s_pos = B->selecter_pos;
    int s_row = s_pos / 5;
    int s_col = s_pos % 5;

    int ret, pos;

    int tmp = -10000;
    for (int i = 0; i < 5; i++) {

        if (B->turn == P1_TURN) {
            // 横一列のmaxを探す
            pos = i + s_row * 5;
            if (i == s_col || B->state[pos] == B_NON) {
                continue;
            }
        } else {
            // 縦一列のmaxを探す
            pos = i * 5 + s_col;
            if (i == s_row || B->state[pos] == B_NON) {
                continue;
            }
        }
        if (B->piece[pos] > tmp) {
            tmp = B->piece[pos];
            ret = pos;
        }
    }
    return ret;
}

int min_max_simple() {
    int ret_pos;

    int *legal_moves = get_legal_moves();
    int val_MAX = -10000;
    for (int i = 0; i < legal_moves[0]; i++) {
        int val = 0;

        int pos = legal_moves[i+1];

        push_pos(pos);

        val += B->piece[B->selecter_pos];

        if (is_gameover()) {
            // if my move induces gameover and necessary win,
            // terminate the GAME(early retire).
            if (val + B->P1_point > B->P2_point) {
                return pos;
            }

            if (val > val_MAX) {
                val_MAX = val;
                ret_pos = pos;
            }
            pop();
            continue;
        }

        int pos_0 = get_imm_opt_move();
        val -= B->piece[pos_0];
        if (val > val_MAX) {
            val_MAX = val;
            ret_pos = pos;
        }
        pop();
    }

    return ret_pos;
}

void next_state() {
    if (B->turn == P1_TURN) {
        char move[2];
        printf("move: ");
        scanf("%s", move);
        int m_pos = move2pos(move);
        while (!is_legal_move(m_pos)) {
            printf("invalid move, choose again: ");
            scanf("%s", move);
            m_pos = move2pos(move);
        }

        B->P1_point += B->piece[m_pos];
        push_pos(m_pos);
    } else {
        //int com_move_pos = get_imm_opt_move();
        int com_move_pos = min_max_simple();

        char com_move[2];
        pos2move(com_move_pos, com_move);
        printf("COM move: %s\n", com_move);

        B->P2_point += B->piece[com_move_pos];
        push_pos(com_move_pos);
    }
}

int main() {
    B = malloc(sizeof(struct BOARD));
    init();

    while (1) {
        print_board();

        if (is_gameover()){
            printf("game over\n");
            print_result();
            break;
        }

        next_state();
    }

    return 0;
}
