#include "pollfd.h"

#include "saturnd.h"

int get_req_pipe() {
    int fd;
    if ((fd = open(REQ_PIPE_PATH, O_RDONLY | O_NONBLOCK) < 0)) {
        perror("Error when opening request pipe");
        exit(EXIT_FAILURE);
    }
    return fd;
}

struct pollfd* get_fds(int self_pipe_stdin) {
    struct pollfd* fds = calloc(2, sizeof(struct pollfd));

    fds[0].fd = get_req_pipe();
    fds[0].events = POLLIN;

    fds[1].fd = self_pipe_stdin;
    fds[1].events = POLLIN;
    return fds;
}