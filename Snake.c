#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ROW 5 // min: 10
#define COL 18 // min: 30, has to be plural

#define GAME_ROW (ROW - 2)
#define GAME_COL ((COL - 2) / 2)

typedef struct Snakes{
    int row, col, heading; // row & col are virtual coord that corresponds to game board
    struct Snakes* next;
} Snake;

typedef struct {
    int row, col;
} Coord;

enum {
    LEFT = 260, RIGHT = 261, UP = 259, DOWN = 258
};

enum {
    CYAN = 1, BLACK_YELLOW, WHITE, BLACK_MAGENTA, BLACK, RED_CYAN, BLUE
};

enum {
    EMPTY = 0, TAIL, FOOD, BODY, OBSTACLE   // TODO: 1. add walls as the game goes on which serve as OBSTACLEs
};

bool init_scr();
void init_arena(WINDOW*);
void init_colour();
Snake init_snake(WINDOW*, int[GAME_ROW][GAME_COL]);
void grow(Snake*, int[GAME_ROW][GAME_COL], int*);
void display_length(int);
void display_snake(Snake*, WINDOW*);
void move_snake(Snake*, int, int[GAME_ROW][GAME_COL]);
int turn(int, Snake*, int[GAME_ROW][GAME_COL], int* );
void game_over(WINDOW*);
Coord create_food(int[GAME_ROW][GAME_COL], int(*)());
void display_food(WINDOW*, Coord);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./snake -lv[1~10]  eg: ./snake -lv5\n");
        return 1;
    }
    int level;
    int count = sscanf(argv[1], "-lv%d", &level);
    if (count != 1 || level < 1 || level > 10) {
        printf("Usage: ./snake -lv[1~10]  eg: ./snake -lv5\n");
        return 1;
    }
    if (!init_scr()) return 1;
    WINDOW* game = newwin(ROW - 2, COL - 2, 1, 1);
    init_arena(game);
    int length = 4;
    display_length(length);
    int board[ROW - 2][(COL - 2) / 2] = {EMPTY};
    Snake head = init_snake(game, board);
    display_snake(&head, game);
    srand((unsigned)time(NULL));
    int (*get_rand)() = rand;
    Coord coord = create_food(board, get_rand);
    display_food(game, coord);
    wgetch(game);
    int ch = getch();
    while(1) {

        if (!((ch == LEFT && (head.heading == RIGHT || head.heading == LEFT)) ||
              (ch == RIGHT && (head.heading == LEFT || head.heading == RIGHT)) ||
              (ch == UP && (head.heading == DOWN || head.heading == UP)) ||
              (ch == DOWN && (head.heading == UP || head.heading == DOWN)))) usleep((useconds_t)(500000 - (level - 1) * 50000));
        if ((ch < 0) ||
            ((ch == LEFT && head.heading == RIGHT) ||
            (ch == RIGHT && head.heading == LEFT) ||
            (ch == UP && head.heading == DOWN) ||
            (ch == DOWN && head.heading == UP))) ch = head.heading;
        if (ch == 'q') break;
        nodelay(stdscr, TRUE);
        nodelay(game, TRUE);
        ch = getch();//      2. snake go though food
        int result = turn(ch, &head, board, &length);
        werase(game);
        if (result == FOOD) coord = create_food(board, get_rand);
        display_snake(&head, game);
        display_food(game, coord);
        display_length(length);
        if (result == OBSTACLE) break;
    }
    game_over(game);
    endwin();
    return 0;
}

void game_over(WINDOW* game) {
    int x, y;
    getmaxyx(game, y, x);
    wattron(game, A_STANDOUT | A_BOLD | A_BLINK | COLOR_PAIR(RED_CYAN));
    mvwprintw(game, y / 2 - 1, x / 2 - 9, "G A M E   O V E R\n");
    mvwprintw(game, y / 2, x / 2 - 10, "(press 'q' to quit)\n");
    wattrset(game, A_NORMAL);
    wrefresh(game);
    while (1) {
        if (getch() == 'q') break;
    }
}

int turn(int key, Snake* head, int board[GAME_ROW][GAME_COL], int* length) {
    switch (key) {
        case LEFT:
            if (head->col != 0 && board[head->row][head->col - 1] <= FOOD) {
                if (board[head->row][head->col - 1] == FOOD) {
                    grow(head, board, length);
                    board[head->row][head->col - 1] = EMPTY;
                    move_snake(head, LEFT, board);
                    return FOOD;
                }
                move_snake(head, LEFT, board);
            } else return OBSTACLE;
            break;
        case RIGHT:
            if (head->col != GAME_COL - 1 && board[head->row][head->col + 1] <= FOOD) {
                if (board[head->row][head->col + 1] == FOOD) {
                    grow(head, board, length);
                    board[head->row][head->col + 1] = EMPTY;
                    move_snake(head, RIGHT, board);
                    return FOOD;
                }
                move_snake(head, RIGHT, board);
            } else return OBSTACLE;
            break;
        case UP:
            if (head->row != 0 && board[head->row - 1][head->col] <= FOOD) {
                if (board[head->row - 1][head->col] == FOOD) {
                    grow(head, board, length);
                    board[head->row - 1][head->col] = EMPTY;
                    move_snake(head, UP, board);
                    return FOOD;
                }
                move_snake(head, UP, board);
            } else return OBSTACLE;
            break;
        case DOWN:
            if (head->row != GAME_ROW - 1 && board[head->row + 1][head->col] <= FOOD) {
                if (board[head->row + 1][head->col] == FOOD) {
                    grow(head, board, length);
                    board[head->row + 1][head->col] = EMPTY;
                    move_snake(head, DOWN, board);
                    return FOOD;
                }
                move_snake(head, DOWN, board);
            } else return OBSTACLE;
            break;
        default: break;
    }
    return EMPTY;
}

Coord create_food(int board[GAME_ROW][GAME_COL], int(*get_rand)()) {
    int row, col;
    do {
        row = get_rand() % GAME_ROW;
        col = get_rand() % GAME_COL;
    } while (board[row][col] != EMPTY);
    board[row][col] = FOOD;
    Coord coord;
    coord.row = row;
    coord.col = col;
    return coord;
}

void display_food(WINDOW* game, Coord coord) {
    wattrset(game, COLOR_PAIR(BLUE));
    mvwprintw(game, coord.row, coord.col * 2, "  ");
    wattroff(game, COLOR_PAIR(BLUE));
    wrefresh(game);
}

void display_snake(Snake* head, WINDOW* game) {
    wattrset(game, COLOR_PAIR(WHITE));
    mvwprintw(game, head->row, head->col * 2, "  ");
    wattroff(game, COLOR_PAIR(WHITE));
    Snake* cur = head->next;
    wattrset(game, COLOR_PAIR(BLACK));
    while (cur != NULL) {
        mvwprintw(game, cur->row, cur->col * 2, "  ");
        cur = cur->next;
    }
    wattroff(game, COLOR_PAIR(BLACK));
    wrefresh(game);
}

void display_length(int length) {
    attrset(COLOR_PAIR(BLACK_MAGENTA));
    mvprintw(ROW - 1, COL - 3, "%03d", length);
    attroff(COLOR_PAIR(BLACK_MAGENTA));
    refresh();
}

Snake init_snake(WINDOW* game, int board[GAME_ROW][GAME_COL]) {
    Snake snake;
    int y, x;
    getmaxyx(game, y, x);
    snake.col = x / 2 / 2;
    snake.row = y / 2;
    snake.heading = RIGHT;
    snake.next = NULL;
    Snake* cur = &snake;
    board[snake.row][snake.col] = BODY;
    for (int i = 1; i < 4; i++) {
        Snake* newpart = (Snake*)malloc(sizeof(Snake));
        newpart->col = snake.col - i;
        newpart->row = snake.row;
        cur->next = newpart;
        cur = cur->next;
        board[newpart->row][newpart->col] = i == 3 ? TAIL : BODY;
    }
    return snake;
}

// need to check whether direction is legal first
void move_snake(Snake* head, int direction, int board[GAME_ROW][GAME_COL]) {
    Snake* cur = head->next;
    int temp_row1 = head->row;
    int temp_col1 = head->col;
    int temp_row2, temp_col2;
    while (cur != NULL) {
        temp_row2 = cur->row;
        temp_col2 = cur->col;
        cur->row = temp_row1;
        cur->col = temp_col1;
        temp_row1 = temp_row2;
        temp_col1 = temp_col2;
        if (cur->next == NULL) {
            board[cur->row][cur->col] = TAIL;
        }
        cur = cur->next;
    }
    board[temp_row1][temp_col1] = EMPTY;
    if (direction == RIGHT) head->col++;
    if (direction == LEFT) head->col--;
    if (direction == UP) head->row--;
    if (direction == DOWN) head->row++;
    head->heading = direction;
    board[head->row][head->col] = BODY;
}

// absolutely no idea why this function works
void grow(Snake* head, int board[GAME_ROW][GAME_COL], int* length) {
    (*length)++;
    Snake* cur = head;
    while(cur->next != NULL) {
        cur = cur->next;
    }
    board[cur->row][cur->col] = BODY;
    Snake* newpart = (Snake*)malloc(sizeof(Snake));
    newpart->col = head->col;
    newpart->row = head->row;
    newpart->next = NULL;
    cur->next = newpart;
    board[newpart->row][newpart->col] = TAIL;
}

bool init_scr() {
    bool i = true;
    initscr();
    cbreak();
    if (LINES < ROW || COLS < COL) {
        printw("You need a screen no smaller than");
        attron(A_BOLD);
        printw(" %d * %d ", ROW, COL);
        attroff(A_BOLD);
        printw("to play.\n");
        i = false;
    }
    if (!has_colors()) {
        printw("Your terminal does not support color display.\n");
        i = false;
    }
    if (!i) {
        printw("Press any key to quit.\n");
        refresh();
        getchar();
        endwin();
        return false;
    }
    init_colour();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // comment out this line to make the snake move step by step
    curs_set(0);
    return true;
}

void init_colour() {
    start_color();
    init_pair(CYAN, COLOR_CYAN, COLOR_CYAN);
    init_pair(BLACK_YELLOW, COLOR_BLACK, COLOR_YELLOW);
    init_pair(WHITE, COLOR_WHITE, COLOR_WHITE);
    init_pair(BLACK_MAGENTA, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(RED_CYAN, COLOR_RED, COLOR_BLACK);
    init_pair(BLUE, COLOR_BLUE, COLOR_BLUE);

}

void init_arena(WINDOW* game) {
    wbkgd(stdscr, COLOR_PAIR(WHITE));
    wbkgd(game, COLOR_PAIR(CYAN));
    attrset(COLOR_PAIR(BLACK_YELLOW));
    vline('s', ROW);
    hline('s', COL);
    mvvline(0, COL - 1, 's', ROW - 1);
    mvhline(ROW - 1, 0, 's', COL - 3);
    attroff(COLOR_PAIR(BLACK_YELLOW));
    refresh();
    wrefresh(game);
}
