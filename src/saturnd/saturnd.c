#include "saturnd.h"

int self_pipe[2];
char abs_path[256];
char req_fifo[256];
char res_fifo[256];

void handle_sigchld(__attribute__((unused)) int sig) {
    int savedErrno; /* In case we change 'errno' */

    savedErrno = errno;
    if (write(self_pipe[1], "x", 1) == -1 && errno != EAGAIN) exit(-1);
    errno = savedErrno;
}

void init_paths() {
    struct pipes_paths* paths = get_default_paths();
    strcpy(abs_path, paths->ABS_PATH);
    strcpy(req_fifo, paths->REQ_PATH);
    strcpy(res_fifo, paths->RES_PATH);
    free(paths->REQ_PATH);
    free(paths->RES_PATH);
    free(paths->ABS_PATH);
}

int init_fifos() {
    _mkdir(abs_path);

    if (access(req_fifo, F_OK) != 0) {
        if (mkfifo(req_fifo, 0600) < 0) {
            return -1;
        }
    }

    if (access(res_fifo, F_OK) != 0) {
        if (mkfifo(res_fifo, 0600) < 0) {
            return -1;
        }
    }
    return 0;
}

int get_req_pipe() {
    int fd = open(req_fifo, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Error when opening request pipe");
        exit(EXIT_FAILURE);
    }
    return fd;
}

// Self pipe from man7

void setup_self_pipes() {
    int flags;

    flags = fcntl(self_pipe[0], F_GETFL);
    if (flags == -1) exit(-1);
    flags |= O_NONBLOCK; /* Make read end nonblocking */
    if (fcntl(self_pipe[0], F_SETFL, flags) == -1) exit(-1);

    flags = fcntl(self_pipe[1], F_GETFL);
    if (flags == -1) exit(-1);
    flags |= O_NONBLOCK; /* Make write end nonblocking */
    if (fcntl(self_pipe[1], F_SETFL, flags) == -1) exit(-1);
}

int main() {
    // Create the daemon
    create_daemon();

    // Reset umask
    umask(0000);

    // Create paths
    init_paths();

    // Init fifos
    if ((init_fifos()) < 0) {
        perror("Error when initiating fifos");
        goto error;
    }

    // Handle SIGCHLD signal
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    // Create self pipe (self pipe trick)
    if (pipe(self_pipe) < 0) {
        perror("Self Pipe error");
        goto error;
    }

    setup_self_pipes();

    // Poll will check on request pipe and self_pipe
    struct pollfd* fds = get_fds(get_req_pipe(), self_pipe[0]);

    while (1) {
        // Wait for 1 minute
        int polled = poll(fds, 2, 1000 * 60);

        // There's an error
        if (polled < 0) {
            printf("A process just ran (SIGCHLD signal recieved)\n");
            // perror("Poll error");
            // goto error;
        }

        // Timeout
        else if (polled == 0) {
            // int r = handle_check_tasks();
            // if (r < 0) {
            //     perror("Checking tasks failed");
            //     goto error;
            // }
            int r = handle_check_tasks();
            if (r < 0) {
                perror("handle check tasks");
                return -1;
            }
            // handle_run_task(2);

        }

        // One or more fds recieved an event
        else {
            for (int i = 0; i < 2; i++) {
                char* buf = malloc(BUFSIZ);

                // Events actually occured
                if (fds[i].revents != 0) {
                    // Event is POLLIN
                    if (fds[i].revents & POLLIN) {
                        int bytes_read = read(fds[i].fd, buf, BUFSIZ);

                        // It's not the self-pipe
                        if (i == 0) {
                            // And we actually read some data
                            if (bytes_read < 0) {
                                perror("Read error");
                                goto error;
                            }
                            int r = handle_operation(buf);
                            // We asked saturnd to close (TERMINATE)
                            if (r == 1) {
                                goto cleanup;
                            }
                            // Saturnd error
                            if (r < 0) {
                                perror("FATAL ERROR: Saturnd. Exiting");
                                goto error;
                            }
                        }
                    }

                    // Event is not pollin, probably POLLHUP. Close and open.
                    else {
                        close(fds[i].fd);
                        fds[i].fd = get_req_pipe();
                    }
                }
            }
        }
    }

    goto cleanup;

cleanup:
    free(fds);
    close(self_pipe[0]);
    close(self_pipe[1]);
    return EXIT_SUCCESS;

error:
    if (errno != 0) perror("main");
    free(fds);
    close(self_pipe[0]);
    close(self_pipe[1]);
    return EXIT_FAILURE;
}