#include <windows.h>
#include <stdio.h>

// Fonction pour afficher des infos système
void print_system_info(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    
    printf("System Information:\n");
    printf("  Processor Architecture: %u\n", si.wProcessorArchitecture);
    printf("  Number of Processors: %u\n", si.dwNumberOfProcessors);
    printf("  Page Size: %u\n", si.dwPageSize);
}

// Fonction avec du code à chiffrer
int calculate_sum(int a, int b)
{
    int result = 0;
    for (int i = a; i <= b; i++)
    {
        result += i;
    }
    return result;
}

// Fonction avec des boucles
void print_pattern(void)
{
    printf("\nPattern:\n");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            printf("* ");
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    printf("=================================\n");
    printf("  Woody Woodpacker PE Test\n");
    printf("=================================\n\n");
    
    printf("Hello from Windows PE test binary!\n");
    printf("This is a test program for woody_woodpacker.\n\n");
    
    // Arguments
    printf("Command line arguments: %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("  argv[%d] = %s\n", i, argv[i]);
    }
    printf("\n");
    
    // Infos système
    print_system_info();
    printf("\n");
    
    // Tests de calcul (code .text)
    printf("Testing code execution:\n");
    int sum = calculate_sum(1, 10);
    printf("  Sum(1..10) = %d\n", sum);
    
    // Pattern
    print_pattern();
    
    printf("\n=================================\n");
    printf("  Test completed successfully!\n");
    printf("=================================\n");
    
    return 0;
}
