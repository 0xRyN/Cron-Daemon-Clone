#include "saturnd-get-stderr.h"

char output[BUFSIZ];

int stderr_to_cassini(int hasFailed, int taskDoesntExist) {
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

        printf("Writing ERROR to cassini\n");

        int w = write(res_fd, buf, 4);
        if (w < 0) {
            perror("Error writing error code");
            return -1;
        }
    }

    // Command hasn't failed
    else {
        // Put replycode in buffer
        int offset = 0;
        char res_buf[BUFSIZ];
        uint16_t reptype = SERVER_REPLY_OK;
        reptype = htobe16(reptype);
        memcpy(res_buf, &reptype, 2);
        offset += 2;

        // Read stderr and put it in buffer

        // First the length of the string
        uint32_t len = strlen(output);
        memcpy(res_buf + offset, &len, 4);
        offset += 4;

        memcpy(res_buf + offset, output, len);
        offset += len;

        int w = write(res_fd, res_buf, offset);
        if (w < 0) {
            perror("Write to stderr");
            return -1;
        }
    }

    close(res_fd);

    return 0;
}

int handle_get_stderr(char *buf) {
    uint64_t taskid;
    memcpy(&taskid, buf, 8);
    buf += 8;
    taskid = be64toh(taskid);

    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%ld/", get_username(), taskid);

    // Task directory doesn't exit
    if (access(path, F_OK) != 0) {
        return stderr_to_cassini(1, 1);
    }

    strcat(path, "stderr");

    // stderr file not initialized (Task not run)
    if (access(path, F_OK) != 0) {
        return stderr_to_cassini(1, 0);
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Fatal error : can't open file");
        return -1;
    }

    // We consider max output = BUFSIZ

    int bytes_read = read(fd, output, BUFSIZ);
    output[bytes_read] = '\0';

    close(fd);

    // Now, we write the output to cassini
    return stderr_to_cassini(0, 0);
}