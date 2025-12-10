#include <stdio.h>

// Fonction avec beaucoup de NOPs pour forcer la compression
void padding_function(void) {
    __asm__(
        "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
    );
}

int main(void) {
    padding_function();
    printf("Hello with compression!\n");
    return 0;
}
