#include "operations.h"

int nb_tasks = -1;

// Will setup all necessary pre-requisites (in case saturnd has shut down)
int init() {
    // Path to tasks
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks", get_username());

    // HOW IT WORKS : Will check all dirs' names for a maximum.

    if (access(path, F_OK) != 0) {
        int r = mkdir(path, 0777);
        if (r < 0) {
            perror("Mkdir");
            return -1;
        }
    }

    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        return -1;
    }
    struct dirent *entry;

    int max = 0;

    while ((entry = readdir(dirp))) {
        // If its not . or ..
        if ((entry->d_name)[0] != '.') {
            int cur = (int)strtol((entry->d_name), NULL, 10);
            if (cur > max) max = cur;
        }
    }

    nb_tasks = max;
    return 0;
}

int handle_operation(char *b) {
    // After launching saturnd, opcode will be -1.
    // We need to reset it to the max tasks' value with init()
    if (nb_tasks == -1) {
        int r = init();
        if (r < 0) {
            perror("init");
            return -1;
        }
    }

    // Find opcode and give it to the switch()
    char *buf = b;
    uint16_t opcode;
    memcpy(&opcode, buf, 2);
    opcode = be16toh(opcode);
    buf += 2;

    // Handle each opcode accordingly
    switch (opcode) {
        // Perfect
        case CLIENT_REQUEST_CREATE_TASK:

            return handle_create_task(buf, nb_tasks);

            break;

        // Perfect
        case CLIENT_REQUEST_REMOVE_TASK:

            return handle_remove_task(buf);

            break;

        // Perfect
        case CLIENT_REQUEST_GET_STDOUT:

            return handle_get_stdout(buf);

            break;

        // Perfect
        case CLIENT_REQUEST_GET_STDERR:

            return handle_get_stderr(buf);

            break;

        // Perfect
        case CLIENT_REQUEST_TERMINATE:

            return 1;

            break;

        // Small error - one extra char sometimes
        case CLIENT_REQUEST_LIST_TASKS:

            return handle_list_tasks();

            break;

        // Perfect
        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES:

            return handle_get_time_exitcode(buf);

            break;

        default:
            printf("Operation is : %x\n", opcode);
            printf("never ever\n");
    }

    return 0;
}