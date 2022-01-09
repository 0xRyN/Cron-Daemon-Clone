#ifndef POLLFD_H
#define POLLFD_H

#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>

struct pollfd* get_fds(int req_fd, int self_pipe_stdin);
int get_req_pipe();

#endif