#include "saturnd-check-tasks.h"

int handle_task(int taskid) {
    // Fill the struct timing
    struct timing *tm = malloc(sizeof(struct timing));
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%d/timing", get_username(), taskid);
    int r1 = get_timing_from_file(tm, path);
    if (r1 < 0) {
        perror("Get timing error");
        return -1;
    }

    // Check if task should run
    int should_run = should_run_now(tm);
    if (should_run) {
        return handle_run_task(taskid);
    }

    else
        return 0;
}

int handle_check_tasks() {
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks", get_username());

    // Tasks dir doesn't exist
    if (access(path, F_OK) != 0) {
        printf("Not a single task...\n");
        return 0;
    }

    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        return -1;
    }
    struct dirent *entry;

    while ((entry = readdir(dirp))) {
        // If its not . or ..
        if ((entry->d_name)[0] != '.') {
            return handle_task(strtol((entry->d_name), NULL, 10));
        }
    }

    return 0;
}