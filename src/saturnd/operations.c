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
            printf("%s\n", entry->d_name);
        }
    }

    nb_tasks = max;
    printf("%i", max);
    return 0;
}

int terminate() {
    // Terminate cleanly
    return 0;
}

int handle_operation(char *b) {
    char *buf = b;
    uint16_t opcode;
    memcpy(&opcode, buf, 2);
    opcode = be16toh(opcode);
    printf("%x", opcode);
    buf += 2;
    // Just after termination
    if (nb_tasks == -1) {
        int r = init();
        if (r < 0) {
            perror("init");
            return -1;
        }
    }
    switch (opcode) {
        case CLIENT_REQUEST_CREATE_TASK:
            return handle_create_task(buf, nb_tasks);
            break;
        default:
            printf("never ever\n");
    }

    return 0;
}