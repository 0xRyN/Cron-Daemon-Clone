#include "saturnd-list-tasks.h"

char abs_path[256];
char req_fifo[256];
char res_fifo[256];

char buf[BUFSIZ];

int index_arr[100];

// Function for quicksort
int compare(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

int count_tasks() {
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks", get_username());

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
            index_arr[nbtasks] = strtol((entry->d_name), NULL, 10);
            nbtasks++;
        }
    }

    return nbtasks;
}

int list_to_cassini(int off) {
    int res_fd = open(res_fifo, O_WRONLY);
    if (res_fd < 0) {
        perror("Error openint res pipe");
        return -1;
    }

    int bytes_written = write(res_fd, buf, off);
    if (bytes_written < 0) {
        perror("Error writing to res pipe from list");
        return -1;
    }

    close(res_fd);

    return 0;
}

int handle_list_tasks() {
    // GLOBAL OFFSET
    int offset = 0;

    // Write OK

    uint16_t okcode = SERVER_REPLY_OK;
    okcode = htobe16(okcode);

    memcpy(buf + offset, &okcode, 2);
    offset += 2;

    // Write NB Tasks

    int nbtasks = count_tasks();
    if (nbtasks < 0) {
        perror("Count tasks");
        return -1;
    }

    uint32_t res_nbtasks = htobe32(nbtasks);

    memcpy(buf + offset, &res_nbtasks, 4);
    offset += 4;

    // Sort index array
    // The idea of a millénaire !!!
    // Since readdir doesn't guarantee alphabetic order,
    // and we need to send to send first task first,
    // First iterate over all the dir
    // Put all the indexes of the directories in an int array
    // Sort the array
    // PROFIT!
    qsort(index_arr, nbtasks, sizeof(int), compare);

    for (int i = 0; i < nbtasks; i++) {
        int index = index_arr[i];
        // ----------------------------------------//
        //                   Taskid                //
        // ----------------------------------------//

        // OPEN TASKID
        char taskid_path[256];
        sprintf(taskid_path, "/tmp/%s/saturnd/tasks/%i/taskid", get_username(),
                index);

        // FILL TASKID
        uint64_t *tid = malloc(sizeof(uint64_t));
        int tid_res = get_taskid_from_file(tid, taskid_path);

        if (tid_res < 0) {
            perror("tid from file");
            return -1;
        }

        *tid = htobe64(*tid);

        memcpy(buf + offset, tid, 8);
        offset += 8;

        free(tid);

        // ----------------------------------------//
        //                   Timing                //
        // ----------------------------------------//

        // OPEN TIMING
        char timing_path[256];
        sprintf(timing_path, "/tmp/%s/saturnd/tasks/%i/timing", get_username(),
                index);

        // FILL TIMING
        struct timing *tm = malloc(sizeof(struct timing));
        int timing_res = get_timing_from_file(tm, timing_path);

        if (timing_res < 0) {
            perror("Timing from file");
            return -1;
        }

        tm->minutes = htobe64(tm->minutes);
        tm->hours = htobe32(tm->hours);

        memcpy(buf + offset, &(tm->minutes), 8);
        offset += 8;

        memcpy(buf + offset, &(tm->hours), 4);
        offset += 4;

        memcpy(buf + offset, &(tm->daysofweek), 1);
        offset += 1;

        free(tm);

        // ----------------------------------------//
        //                   CMDS-                //
        // ----------------------------------------//

        // OPEN COMMAND
        char command_path[256];
        sprintf(command_path, "/tmp/%s/saturnd/tasks/%i/command",
                get_username(), index);

        // FILL COMMAND
        struct command *cmd = malloc(sizeof(struct command));
        int command_res = get_command_from_file(cmd, command_path);

        if (command_res < 0) {
            perror("command from file err");
            return -1;
        }

        uint32_t res_argc = htobe32(cmd->argc);

        memcpy(buf + offset, &res_argc, 4);
        offset += 4;

        for (int i = 0; i < (int)(cmd->argc); i++) {
            struct cstring *iterator = cmd->argv;
            uint32_t res_len = htobe32(iterator[i].length+1);
            memcpy(buf + offset, &res_len, 4);
            offset += 4;

            memcpy(buf + offset, iterator[i].value, iterator[i].length+1);
            offset += iterator[i].length+1;
        }
    }
    return list_to_cassini(offset);
}