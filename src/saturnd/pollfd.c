#include "pollfd.h"

#include "saturnd.h"

struct pollfd* get_fds(int req_fd, int self_pipe_stdin) {
    struct pollfd* fds = calloc(2, sizeof(struct pollfd));

    fds[0].fd = req_fd;
    fds[0].events = POLLIN;

    fds[1].fd = self_pipe_stdin;
    fds[1].events = POLLIN;
    return fds;
}