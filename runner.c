#define _POSIX_C_SOURCE	199309L
#define _XOPEN_SOURCE 500

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 12
#define TICK 100000
#define BOARD_SPEED 5

#define E_INVALID_OBJECT_IN_BOARD 2

#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <ncurses.h>

int tick_num = 0;

struct board {
    WINDOW * window;
    int * player_row;
    unsigned int score;
    unsigned int level;
    int ** rows;
    unsigned int row_count;
    unsigned int running;
};

int get_bit(int * data, int element)
{
    unsigned int byte_index = element/8;
    unsigned int bit_index = element % 8;
    unsigned int bit_mask = ( 1 << bit_index);

    return ((data[byte_index] & bit_mask) != 0);
}

void set_bit (int * data, int element)
{
    unsigned int byte_index = element/8;
    unsigned int bit_index = element % 8;
    unsigned int bit_mask = ( 1 << bit_index);

    data[byte_index] |= bit_mask;
}

void clear_bit (int * data, int element)
{
    unsigned int byte_index = element/8;
    unsigned int bit_index = element % 8;
    unsigned int bit_mask = ( 1 << bit_index);

    data[byte_index] &= ~bit_mask;
}

int* new_row(int set) {
    int * new_row;
    new_row = malloc((BOARD_WIDTH/8) + 1);

    set_bit(new_row, set);

    return new_row;
}

void print_bin(WINDOW *win, const char *label, unsigned int value) {
    wprintw(win, "%s: ", label);
    for (int i = BOARD_WIDTH - 1; i >= 0; i--) {
        waddch(win, (value & (1u << i)) ? '1' : '0');
    }
    waddch(win, '\n');
}

void update_objects(struct board* board) {
    /* unsigned char temp = (*board).rows[BOARD_HEIGHT-1]; */
    for(int i = BOARD_HEIGHT; i > 0; i--) {
        (*board).rows[i] = (*board).rows[i-1];
    }
    /* (*board).rows[0] = temp; //this wraps the bottom to the top */

    int * new = new_row(0);
    clear_bit(new, 0);
    for (int i = 0; i < (*board).level; i++) {
        int r = rand() % BOARD_WIDTH;
        set_bit(new,  r);
    }

    (*board).rows[0] = new;
}

void update_player(struct board* board) {
    cbreak(); 
    nodelay(stdscr, TRUE);
    int ch;
    if ((ch = getch()) != ERR) {
        if (ch == KEY_LEFT) {
            if ((*board->player_row) != 0b00000001) {
                (*board->player_row) = (*board->player_row) >> 1;
            }
        }

        if (ch == KEY_RIGHT) {
            if ((*board->player_row) != 0b10000000) {
                (*board->player_row) = (*board->player_row) << 1;
            }
        }
    }
    flushinp();
}

void update(struct board* board) {
    for (int i = 0; i < BOARD_WIDTH-1; i++) {
        int board_bit = get_bit(board->rows[BOARD_HEIGHT-1], i);
        int player_bit = get_bit(board->player_row, i);

        if (board_bit && player_bit) {
            (*board).running = 0;
            return;
        }
    }

    update_player(board);
    if (tick_num % BOARD_SPEED == 0) {
        update_objects(board);
        (*board).score++;

        if ((*board).score % 50 == 0) {
            (*board).level++;
        }
    }
}

int draw(struct board board) {
    clear();
    wclear(board.window);

    printw("--RUNNER--\n");

    waddch(board.window, ' '); //border
    for (int i = 0; i <= BOARD_HEIGHT-2; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {  
            int board_bit = get_bit(board.rows[i], j);
            if (board_bit) {
                waddch(board.window, '@');
            } else {
                waddch(board.window, ' ');
            }
        }
        waddch(board.window, '\n');
        waddch(board.window, ' '); //border
    }

    for (int i = 0; i < BOARD_WIDTH; i++) {
        int board_bit = get_bit(board.rows[BOARD_HEIGHT-1], i);
        int player_bit = get_bit(board.player_row, i);

        if (board_bit && player_bit) {
            waddch(board.window, 'X');
        } else if (player_bit) {
            waddch(board.window, '^');
        } else if (board_bit) {
            waddch(board.window, '@');
        } else {
            waddch(board.window, ' ');
        }
    }
    waddch(board.window, '\n');

    for (int i = 0; i <= BOARD_HEIGHT; i++) {
        printw("\n");
    }
    printw("SCORE: %d\n", board.score);
    printw("LEVEL: %d\n", board.level);
    /* print_bin(stdscr, "PLAYER_POS", (*board.player_row)); */

    wborder(board.window, 0, 0, 0, 0, 0, 0, 0, 0);
    refresh();
    wrefresh(board.window);

    return 0;
}

void free_board(struct board *board) {
    if (board->window) {
        delwin(board->window);
        board->window = NULL;
    }

    if (board->player_row) {
        free(board->player_row);
        board->player_row = NULL;
    }

    for (int i = 0; i < board->row_count; i++) {
        if (board->rows[i]) {
            free(board->rows[i]);
            board->rows[i] = NULL;
        }
    }
}

int main() {
    initscr();
    noecho(); 
    curs_set(0);
    keypad(stdscr, TRUE); 

    WINDOW * win = newwin(BOARD_HEIGHT+1, BOARD_WIDTH+3, 1, 0);
    keypad(win, TRUE); 

    box(win, 0, 0);
    wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);
    wrefresh(win);
    refresh();

    unsigned int row_count = BOARD_HEIGHT;

    int ** rows = malloc(sizeof(int *) * row_count);
    for (int i = 0; i < row_count; i++) {
        int * row = new_row(0);
        clear_bit(row, 0);
        rows[i] = row;
    }

    struct board board = {
        .window = win,
        .player_row = new_row(4),
        .score = 0,
        .row_count = row_count,
        .rows = rows,
        .running = 1,
        .level = 1,
    };

    do {
        draw(board);
        update(&board);

        usleep(TICK);
        tick_num++;
    }while (board.running);

    free_board(&board);
}
