#include "saturnd.h"

int req_fifo;
int res_fifo;
int self_pipe[2];

void handle_sigchld(__attribute__((unused)) int sig) {
    if ((write(self_pipe[1], "a", 1)) < 0) {
        perror("Error when writing to self pipe");
        exit(EXIT_FAILURE);
    }
}

void check_tasks() {
    int fd = open("test-res", O_WRONLY);
    write(fd, "no", 2);
}

int init_paths() {
    char user[20];
    user = getenv("USER");
}

int init_fifos() {
    if (mkfifo(REQ_PIPE_PATH, 0600) < 0) {
        return -1;
    }

    if (mkfifo(REQ_PIPE_PATH, 0600) < 0) {
        return -1;
    }

    return 0;
}

int main() {
    // Create the daemon
    create_daemon();

    // Reset umask
    umask(0000);

    // Create paths

    // Init fifos
    if ((init_fifos()) < 0) {
        perror("Error when initiating fifos");
        goto error;
    }

    // Handle SIGCHLD signal
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sigaction(SIGCHLD, &sa, NULL);

    if (pipe(self_pipe) < 0) {
        perror("Self Pipe error");
        goto error;
    }

    // Poll will check on request pipe and self_pipe
    struct pollfd* fds = get_fds(self_pipe[0]);

    while (1) {
        // Wait for 1 minute
        int polled = poll(fds, 2, 10000);

        // There's an error
        if (polled < 0) {
            perror("Poll error");
            goto error;
        }

        // Timeout
        else if (polled == 0) {
            // check_tasks();
        }

        // One or more fds recieved an event
        else {
            for (int i = 0; i < 2; i++) {
                char buf[BUFSIZ];

                if (fds[i].revents & POLLIN) {
                    int bytes_read = read(fds[i].fd, buf, sizeof(buf));
                    buf[bytes_read] = '\0';

                    // We are reading the req pipe
                    if (i == 0) {
                        // int res_fd;
                        // if ((res_fd = open(RES_PIPE, O_WRONLY)) < 0) {
                        //     perror("error");
                        //     goto error;
                        // }
                        // write(res_fd, buf, bytes_read);
                        check_tasks();
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