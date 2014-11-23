#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "args.h"

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

