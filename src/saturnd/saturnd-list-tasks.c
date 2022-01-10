#include "saturnd-list-tasks.h"



int handle_list_tasks() {
    FILE *fptr;
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
            //printf("%s",(entry->d_name));
            count++;
        }
    }
    int fd = fopen("/tmp/rayane/saturnd/tasks/2/command", "rb");
    struct stat st;
    stat("/tmp/rayane/saturnd/tasks/2/command", &st);
    int sizeB = st.st_size;
    char c = fgetc(fptr);
    char buff[sizeB];
   
    return count;
}