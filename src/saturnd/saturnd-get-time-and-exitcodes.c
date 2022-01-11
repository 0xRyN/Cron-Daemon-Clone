#include "saturnd-get-time-and-exitcodes.h"

char abs_path[256];
char req_fifo[256];
char res_fifo[256];

char buf[BUFSIZ];

int index_arr[100];


int count_MaxTaskFromId (int taskId) {
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
            index_arr[nbtasks] = strtol((entry->d_name), NULL, 10);
            nbtasks++;
        }
    }

    return nbtasks;
}

int time_to_cassini(int off) {
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
