#include "saturnd-create-task.h"

int nb_tasks;
char *buf;
char abs_path[256];
char req_fifo[256];
char res_fifo[256];

// Create taskid file

int handle_taskid(char *path) {
    char taskid_path[256];
    strcpy(taskid_path, path);
    strcat(taskid_path, "/taskid");

    int taskid_fd = open(taskid_path, O_CREAT | O_WRONLY, 0777);
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

// Create timing file

int handle_timing(char *path) {
    char timing_path[256];
    strcpy(timing_path, path);
    strcat(timing_path, "/timing");

    int timing_fd = open(timing_path, O_CREAT | O_WRONLY, 0777);
    if (timing_fd < 0) {
        perror("Open timing");
        return -1;
    }

    struct timing tm;

    memcpy(&(tm.minutes), buf, 8);
    buf += 8;
    tm.minutes = be64toh(tm.minutes);

    memcpy(&(tm.hours), buf, 4);
    buf += 4;
    tm.hours = be32toh(tm.hours);

    memcpy(&(tm.daysofweek), buf, 1);
    buf += 1;

    int w_timing = write(timing_fd, &tm, 8 + 4 + 1);
    if (w_timing < 0) {
        perror("Write to timing file");
        return -1;
    }

    close(timing_fd);

    return 0;
}

// Create command file

int handle_command(char *path) {
    char command_path[256];
    strcpy(command_path, path);
    strcat(command_path, "/command");

    int command_fd = open(command_path, O_CREAT | O_WRONLY, 0777);
    if (command_fd < 0) {
        perror("Open command");
        return -1;
    }

    // Giant buffer + Offset for one write syscall
    char command_buffer[BUFSIZ];
    int offset = 0;

    // First read argc, and put it in the buffer
    uint32_t cmd_argc;
    memcpy(&cmd_argc, buf, 4);
    buf += 4;
    cmd_argc = be32toh(cmd_argc);

    memcpy(command_buffer + offset, &cmd_argc, 4);
    offset += 4;

    for (int i = 0; i < (int)cmd_argc; i++) {
        // Read length

        uint32_t argv_len;
        memcpy(&argv_len, buf, 4);
        buf += 4;
        argv_len = be32toh(argv_len);

        memcpy(command_buffer + offset, &argv_len, 4);
        offset += 4;

        // Read the arg
        char arg[argv_len];
        memcpy(arg, buf, argv_len);
        buf += argv_len;

        memcpy(command_buffer + offset, arg, argv_len);
        offset += argv_len;
    }

    int w_command = write(command_fd, command_buffer, offset);
    if (w_command < 0) {
        perror("Write w_command");
        return -1;
    }

    close(command_fd);

    return 0;
}

// Create runs directory

int handle_runs(char *path) {
    char runs_path[256];
    strcpy(runs_path, path);
    strcat(runs_path, "/runs");

    int r = mkdir(runs_path, 0777);
    if (r < 0) {
        perror("Error creating runs dir");
        return -1;
    }

    return 0;
}

// Send info back to cassini

int create_to_cassini() {
    // Write OK and Taskid
    int res_fd = open(res_fifo, O_WRONLY);
    if (res_fd < 0) {
        perror("Open response pipe");
        return -1;
    }

    char buf[4];
    uint16_t okcode = SERVER_REPLY_OK;
    uint16_t rescode = nb_tasks;

    okcode = htobe16(okcode);
    rescode = htobe16(rescode);

    // Copy buf
    memcpy(buf, &okcode, 2);
    memcpy(buf, &rescode, 2);

    int w = write(res_fd, buf, 4);
    if (w < 0) {
        perror("Error writing error code");
        return -1;
    }

    close(res_fd);

    return 0;
}

int handle_create_task(char *b, int nbtasks) {
    // Setup globals
    buf = b;
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

    create_to_cassini(0);
    return nb_tasks;
}