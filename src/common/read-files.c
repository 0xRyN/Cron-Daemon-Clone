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
    int r1 = read(fd, &minutes, 8);
    if (r1 < 0) {
        perror("fail to read timing minute");
        return -1;
    }
    int r2 = read(fd, &hours, 4);
    if (r2 < 0) {
        perror("fail to read timing hours");
        return -1;
    }
    int r3 = read(fd, &day, 1);
    if (r3 < 0) {
        perror("fail to read timing day");
        return -1;
    }

    time->minutes = (minutes);
    time->hours = (hours);
    time->daysofweek = day;

    close(fd);
    return 0;
}

int get_taskid_from_file(uint64_t *tId, char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("fail to open Task id file");
        return -1;
    }
    int r = read(fd, tId, 8);
    if (r < 0) {
        perror("fail to read id");
        return -1;
    }
    close(fd);
    return 0;
}

int get_command_from_file(struct command *cmd, char *path) {
    int fd = open(path, O_RDONLY);

    // First read the commands number of arguments
    uint32_t cmd_agrc;
    read(fd, &cmd_agrc, 4);
    cmd->argc = cmd_agrc;

    // Argv will contain cmd_argc cstrings
    cmd->argv = malloc(cmd_agrc * sizeof(struct cstring));

    if (cmd->argv == NULL) {
        perror("Failed to alloc argv on command");
        return -1;
    }

    for (unsigned int i = 0; i < cmd_agrc; i++) {
        // First read the length
        uint32_t str_length;
        int r1 = read(fd, &str_length, 4);
        if (r1 < 0) {
            perror("Coudln't read length of strlength");
            return -1;
        }
        // str_length = be32toh(str_length);

        // Alloc a buf of length bytes and read on it

        char *str_data = malloc(str_length);
        int r2 = read(fd, str_data, str_length);
        if (r2 < 0) {
            perror("Coudln't read char data");
            return -1;
        }

        // Allocate a cstring and put data in it

        struct cstring cst;
        cst.length = str_length;
        cst.value = str_data;

        // Adda the cstring to the command
        cmd->argv[i] = cst;
    }
    close(fd);
    return 0;
}
