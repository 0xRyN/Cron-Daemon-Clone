#include "create-task.h"

int nb_tasks;
int req_fd;

int handle_taskid(char *path) {
    char taskid_path[256];
    strcpy(taskid_path, path);
    strcat(taskid_path, "taskid");

    int taskid_fd = open(taskid_path, O_CREAT | O_WRONLY);
    if (taskid_fd < 0) {
        perror("Open taskid");
        return -1;
    }

    int taskid_write = write(taskid_fd, &nb_tasks, 8);
    if (taskid_write < 0) {
        perror("Write taskid");
        return -1;
    }
    close(taskid_fd);

    return 0;
}

int handle_timing(char *path) {
    char timing_path[256];
    strcpy(timing_path, path);
    strcat(timing_path, "timing");

    int timing_fd = open(timing_path, O_CREAT | O_WRONLY);
    if (timing_fd < 0) {
        perror("Open timing");
        return -1;
    }

    struct timing tm;

    int r_minutes = read(req_fd, &(tm.minutes), 8);
    if (r_minutes < 0) {
        perror("Read minutes");
        return -1;
    }

    int r_hours = read(req_fd, &(tm.hours), 4);
    if (r_hours < 0) {
        perror("Read hours");
        return -1;
    }

    int r_days = read(req_fd, &(tm.hours), 1);
    if (r_days < 0) {
        perror("Read days");
        return -1;
    }

    char buf[TIMING_TEXT_MIN_BUFFERSIZE];
    int res = timing_string_from_timing(buf, &tm);
    if (res < 0) {
        perror("Timing conversion");
        return -1;
    }

    int w_timing = write(timing_fd, buf, 8 + 4 + 1);
    if (w_timing < 0) {
        perror("Write to timing file");
        return -1;
    }

    close(timing_fd);

    return 0;
}

int handle_command(char *path) {
    char command_path[256];
    strcpy(command_path, path);
    strcat(command_path, "command");

    int command_fd = open(command_path, O_CREAT | O_WRONLY);
    if (command_fd < 0) {
        perror("Open command");
        return -1;
    }

    // Giant buffer + Offset for one write syscall
    char buf[BUFSIZ];
    int offset = 0;

    // First read argc, and put it in the buffer
    uint32_t cmd_argc;
    int r_argc = read(req_fd, &cmd_argc, 4);
    if (r_argc < 0) {
        perror("Read argc");
        return -1;
    }
    memcpy(buf + offset, &cmd_argc, 4);
    offset += 4;

    for (int i = 0; i < (int)cmd_argc; i++) {
        // Read length

        uint32_t argv_len;
        int r_len = read(req_fd, &argv_len, 4);
        if (r_len < 0) {
            perror("Read len");
            return -1;
        }

        memcpy(buf + offset, &argv_len, 4);
        offset += 4;

        // Read the arg
        char arg[argv_len];
        int r_arg = read(req_fd, arg, argv_len);
        if (r_arg < 0) {
            perror("Read arg");
            return -1;
        }

        memcpy(buf + offset, arg, argv_len);
        offset += argv_len;
    }

    int w_command = write(command_fd, buf, offset);
    if (w_command < 0) {
        perror("Write w_command");
        return -1;
    }

    close(command_fd);

    return 0;
}

int handle_runs(char *path) {
    char runs_path[256];
    strcpy(runs_path, path);
    strcat(runs_path, "runs");

    int r = mkdir(runs_path, 0666);
    if (r < 0) {
        perror("Error creating runs dir");
        return -1;
    }

    return 0;
}

int handle_create_task(int nbtasks, int fd) {
    // Setup globals
    req_fd = fd;
    nb_tasks = nbtasks;
    nb_tasks++;

    // Create directory for new task
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%d", get_username(), nb_tasks);
    _mkdir(path);

    // Inside the directory, create TaskId, Timing, Command

    // ----------------------------------------//
    //                   Taskid                //
    // ----------------------------------------//

    if (handle_taskid(path) < 0) {
        return -1;
    }

    // ----------------------------------------//
    //                   Timing                //
    // ----------------------------------------//

    if (handle_timing(path) < 0) {
        return -1;
    }

    // ----------------------------------------//
    //                   Commds                //
    // ----------------------------------------//

    if (handle_command(path) < 0) {
        return -1;
    }

    // ----------------------------------------//
    //                   Rundir                //
    // ----------------------------------------//

    if (handle_runs(path) < 0) {
        return -1;
    }

    return 0;
}