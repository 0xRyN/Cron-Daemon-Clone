#define _DEFAULT_SOURCE

#include "read-files.h"

int get_timing_from_file(struct timing *time, char *path) {
    uint64_t minutes;
    uint32_t hours;
    uint8_t day;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("fail to open timing file");
        return -1;
    }
    int r = read(fd, &minutes, 8);
    if (r < 0) {
        perror("fail to read timing minute");
        return -1;
    }
    int r = read(fd, &hours, 4);
    if (r < 0) {
        perror("fail to read timing hours");
        return -1;
    }
    int r = read(fd, &day, 1);
    if (r < 0) {
        perror("fail to read timing day");
        return -1;
    }

    time->minutes = be64toh(minutes);
    time->hours = be32toh(hours);
    time->daysofweek = day;

    close(fd);
    return 0;
}

int get_taskid_from_file(uint64_t tId, char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("fail to open Task id file");
        return -1;
    }
    int r = read(fd, &tId, 8);
    if (r < 0) {
        perror("fail to read id");
        return -1;
    }
    close(fd);
    return 0;
}

int get_command_from_file(struct command *cmd, char *path) {
    int fd = open(path, O_RDONLY);
    uint32_t cmd_agrc;
    read(fd, &cmd_agrc, 4);
    cmd_agrc = be32toh(cmd_agrc);
    cmd->argc = cmd_agrc;

    // For each command, display it's info
    for (unsigned int i = 0; i < cmd_agrc; i++) {
        int str_length;
        read(fd, &str_length, 4);
        str_length = be32toh(str_length);
        char *str_data = malloc(str_length);
        read(fd, str_data, str_length);
        struct cstring *cst = malloc(4 + str_length);
        cst->length = be32toh(str_length);
        cst->value = str_data;
        free(str_data);
    }
    close(fd);
    return 0;
}
