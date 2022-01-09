#include "operations.h"

int nb_tasks = -1;

// Will setup all necessary pre-requisites (in case saturnd has shut down)
int init() {
    // Path to tasks
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks", get_username());

    // HOW IT WORKS : Will check all dirs' names for a maximum.

    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        return -1;
    }
    struct dirent *entry;

    int max = 0;

    while ((entry = readdir(dirp))) {
        int cur = (int)strtol((entry->d_name), NULL, 10);
        if (cur > max) max = cur;
    }

    nb_tasks = max;
    return 0;
}

int terminate() {
    // Terminate cleanly
    return 0;
}

int handle_operation(uint16_t opcode, int req_fd) {
    // Just after termination
    if (nb_tasks == -1) init();
    switch (opcode) {
        case CLIENT_REQUEST_CREATE_TASK:
            return handle_create_task(nb_tasks, req_fd);
            break;
    }

    return 0;
}