#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

// Roughly 4 MB. Always ensure that the executable is smaller than this
#define GPG_MAX_SIZE_BYTES 4194304

#define PROBE_THRESHOLD 120ul

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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s GPGpath\n", argv[0]);
        return 1;
    }

    char *gpg_path = argv[1];
    int gpg_fd = open(gpg_path, O_RDONLY);
    if (gpg_fd == -1) {
        perror("open");
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
    spy(gpg_base);

    // Probably never reached because we'll likely just ^C the program. Maybe
    // implement a SIGTERM / SIGINT handler?
    munmap(gpg_base, map_len);
    close(gpg_fd);
    return 0;
}

