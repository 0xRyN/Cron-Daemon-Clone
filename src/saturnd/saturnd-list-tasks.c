#include "saturnd-list-tasks.h"

int handle_list_tasks() {
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks", get_username());

    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        return -1;
    }
    struct dirent *entry;

    int count = 0;

    while ((entry = readdir(dirp))) {
        // If its not . or ..
        if ((entry->d_name)[0] != '.') {
            count++;
        }
    }
    printf("%i\n",count);
    return count;
}