#include <stdio.h>

#define ITERATIONS 1000

unsigned long probe(char *adrs) {
    volatile unsigned long time;

    asm __volatile__(
        "    mfence             \n"
        "    lfence             \n"
        "    rdtsc              \n"
        "    lfence             \n"
        "    movl %%eax, %%esi  \n"
        "    movl (%1), %%eax   \n"
        "    lfence             \n"
        "    rdtsc              \n"
        "    subl %%esi, %%eax  \n"
        : "=a" (time)
        : "c" (adrs)
        : "%esi", "%edx"
    );
    return time;
}

int main(void) {
    char canary;
    unsigned long total_time = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        total_time += probe(&canary);
    }

    printf("Average L1 load time: %lu cycles\n", total_time / ITERATIONS);
    return 0;
}

