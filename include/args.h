#ifndef __FLUSH_RELOAD_ARGS_H__
#define __FLUSH_RELOAD_ARGS_H__

#include <stdbool.h>
#include <stdio.h>

/** Keeps track of arguments passed from terminal */
struct args_st {
    /** File descriptor of GPG binary */
    int gpg_fd;

    /** FILE stream of file containing addresses to probe */
    FILE *addr_file;

    /** FILE stream of output file containing time slot data */
    FILE *out_file;
};

/** Read arguments into the args_st struct. */
bool read_args(struct args_st *arguments, int argc, char *argv[]);

void cleanup_args(struct args_st *arguments);

/** Read addresses to probe into specified buffer from the addr file */
size_t read_addrs(FILE *addr_file, char **addrs, size_t maxlen);

#endif
