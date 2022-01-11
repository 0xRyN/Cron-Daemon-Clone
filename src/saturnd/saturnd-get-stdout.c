#include "saturnd-get-stdout.h"

char output[BUFSIZ];

int stdout_to_cassini(int hasFailed, int taskDoesntExist, int offset) {
    int res_fd;

    res_fd = open(res_fifo, O_WRONLY);
    if (res_fd < 0) {
        perror("Open response pipe");
        return -1;
    }

    // Command has failed
    if (hasFailed) {
        uint16_t reptype = SERVER_REPLY_ERROR;
        reptype = htobe16(reptype);
        // Check why task failed
        uint16_t errcode = (taskDoesntExist) ? SERVER_REPLY_ERROR_NOT_FOUND
                                             : SERVER_REPLY_ERROR_NEVER_RUN;
        errcode = htobe16(errcode);

        char buf[4];
        memcpy(buf, &reptype, 2);
        memcpy(buf + 2, &errcode, 2);

        // printf("Writing ERROR to cassini\n");

        int w = write(res_fd, buf, 4);
        if (w < 0) {
            perror("Error writing error code");
            return -1;
        }
    }

    // Command hasn't failed
    else {
        int w = write(res_fd, output, offset);
        if (w < 0) {
            perror("Write to stdout");
            return -1;
        }
    }

    close(res_fd);

    return 0;
}

int handle_get_stdout(char *buf) {
    uint64_t taskid;
    memcpy(&taskid, buf, 8);
    buf += 8;
    taskid = be64toh(taskid);

    int offset = 0;

    char path[256];
    char *username = get_username();
    sprintf(path, "/tmp/%s/saturnd/tasks/%ld/", username, taskid);
    free(username);
    // Task directory doesn't exit
    if (access(path, F_OK) != 0) {
        return stdout_to_cassini(1, 1, 0);
    }

    strcat(path, "stdout");

    // Stdout file not initialized (Task not run)
    if (access(path, F_OK) != 0) {
        return stdout_to_cassini(1, 0, 0);
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Fatal error : can't open file");
        return -1;
    }

    uint16_t reptype = SERVER_REPLY_OK;
    reptype = htobe16(reptype);
    memcpy(output, &reptype, 2);
    offset += 2;

    // We consider max output = BUFSIZ

    char temp[1024];

    int bytes_read = read(fd, temp, BUFSIZ);

    // offset += bytes_read;

    uint32_t len = htobe32(bytes_read);
    memcpy(output + offset, &len, 4);
    offset += 4;

    memcpy(output + offset, temp, bytes_read);
    offset += bytes_read;

    close(fd);

    // Now, we write the output to cassini
    return stdout_to_cassini(0, 0, offset);
}