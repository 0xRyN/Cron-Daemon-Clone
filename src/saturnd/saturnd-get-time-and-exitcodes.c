#include "saturnd-get-time-and-exitcodes.h"

char abs_path[256];
char req_fifo[256];
char res_fifo[256];

char buf[BUFSIZ];

int get_nbruns(int taskId) {
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%i/runs", get_username(), taskId);

    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        return -1;
    }
    struct dirent *entry;

    uint32_t nbtasks = 0;

    while ((entry = readdir(dirp))) {
        // If its not . or ..
        if ((entry->d_name)[0] != '.') {
            nbtasks++;
        }
    }

    return nbtasks;
}

int iterate(int taskid, int off) {
    int offset = off;
    int n = get_nbruns(taskid);

    uint32_t res_n = htobe32(n);

    memcpy(buf + offset, &res_n, 4);
    offset += 4;

    for (int i = 1; i <= n; i++) {
        char time_path[256];
        sprintf(time_path, "/tmp/%s/saturnd/tasks/%i/runs/%i/time",
                get_username(), taskid, i);

        int time_fd = open(time_path, O_RDONLY);
        if (time_fd < 0) {
            perror("open time");
            return -1;
        }

        uint64_t time;

        int time_bytes_read = read(time_fd, &time, 8);
        if (time_bytes_read < 0) {
            perror("read time");
            return -1;
        }

        time = htobe64(time);

        memcpy(buf + offset, &time, 8);
        offset += 8;

        char exitcode_path[256];
        sprintf(exitcode_path, "/tmp/%s/saturnd/tasks/%i/runs/%i/exitcode",
                get_username(), taskid, i);

        int exitcode_fd = open(exitcode_path, O_RDONLY);
        if (exitcode_fd < 0) {
            perror("open exitcode");
            return -1;
        }

        uint16_t exitcode;

        int exitcode_bytes_read = read(exitcode_fd, &exitcode, 2);
        if (exitcode_bytes_read < 0) {
            perror("read exitcode");
            return -1;
        }

        exitcode = htobe16(exitcode);

        memcpy(buf + offset, &exitcode, 2);
        offset += 2;
    }
    return time_exitcode_to_cassini(0, offset);
}

int time_exitcode_to_cassini(int hasFailed, int offset) {
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

        int w = write(res_fd, buf, offset);
        if (w < 0) {
            perror("Write to response");
            return -1;
        }
    }

    close(res_fd);

    return 0;
}

int handle_get_time_exitcode(char *b) {
    uint64_t taskid;
    memcpy(&taskid, b, 8);
    b += 8;
    taskid = be64toh(taskid);

    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%ld", get_username(), taskid);

    // If path to task doesnt exist
    if (access(path, F_OK) != 0) {
        return time_exitcode_to_cassini(1, 0);
    }

    // GLOBAL OFFSET
    int offset = 0;

    // Write OK

    uint16_t okcode = SERVER_REPLY_OK;
    okcode = htobe16(okcode);

    memcpy(buf + offset, &okcode, 2);
    offset += 2;

    return iterate(taskid, offset);
}