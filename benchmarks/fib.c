#include <stdio.h>
#include <time.h>

int fib(int n)
{
    if (n < 2)
    {
        return n;
    }

    return fib(n - 2) + fib(n - 1);
}

int main()
{
    float start = (float)clock() / CLOCKS_PER_SEC;

    fib(35);

    float end = (float)clock() / CLOCKS_PER_SEC;
    float elapsed = end - start;

    printf("elapsed: %1.2f\n", elapsed);

    return 0;
}