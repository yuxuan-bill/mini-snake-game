#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

void printrand();

int madin() {
//    srand(time(NULL));
//    int (*get_rand)() = rand;
//    int i;
//    do {
//        i = get_rand() % 10;
//        printf("%d", i);
//    } while (i <= 8);
//    printf("%d", i);
    printf("%d", 1624857150 % 8);



}

void printrand(int (*getrand)()) {
    for (int i = 0; i < 10; i++) {
        printf("%d ", getrand() % 20);
    }
    printf("\n");
}