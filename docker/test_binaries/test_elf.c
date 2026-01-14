#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    printf("Hello from test binary!\n");
    printf("This is a simple ELF test program.\n");
    printf("Arguments: %d\n", argc);
    
    if (argc > 1)
    {
        printf("First argument: %s\n", argv[1]);
    }
    
    return (0);
}
