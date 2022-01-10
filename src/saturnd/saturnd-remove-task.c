#include "saturnd-remove-task.h"

char abs_path[256];
char req_fifo[256];
char res_fifo[256];

int remove_to_cassini(int hasFailed) {
    int res_fd;

    if (hasFailed) {
        res_fd = open(res_fifo, O_WRONLY);
        if (res_fd < 0) {
            perror("Open response pipe");
            return -1;
        }

        uint16_t reptype = SERVER_REPLY_ERROR;
        reptype = htobe16(reptype);
        uint16_t errcode = SERVER_REPLY_ERROR_NOT_FOUND;
        errcode = htobe16(errcode);

        char buf[4];
        memcpy(buf, &reptype, 2);
        memcpy(buf, &errcode, 2);

        printf("Writing ERROR to cassini\n");

        int w = write(res_fd, buf, 4);
        if (w < 0) {
            perror("Error writing error code");
            return -1;
        }
    }

    else {
        res_fd = open(res_fifo, O_WRONLY);
        if (res_fd < 0) {
            perror("Open response pipe");
            return -1;
        }

        uint16_t reptype = SERVER_REPLY_OK;
        printf("Writing OK to cassini\n");
        reptype = htobe16(reptype);

        int w = write(res_fd, &reptype, 2);
        if (w < 0) {
            perror("Write to response");
            return -1;
        }

        close(res_fd);

        return 0;
    }

    close(res_fd);
    return 0;
}

int handle_remove_task(char *buf) {
    uint64_t taskid;
    memcpy(&taskid, buf, 8);
    buf += 8;
    taskid = be64toh(taskid);

    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%ld", get_username(), taskid);

    int r = _rmdir(path);
    if (r < 0) {
        perror("Error when removing task (rmdir)");
        return remove_to_cassini(1);
    }

    return remove_to_cassini(0);
}