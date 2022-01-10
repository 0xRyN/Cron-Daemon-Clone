#include "saturnd-check-tasks.h"

int handle_check_tasks() {
    struct timing *tm = malloc(sizeof(struct timing));
    int r1 = get_timing_from_file(tm, "/tmp/rayan/saturnd/tasks/3/timing");
    if (r1 < 0) {
        perror("Get timing error");
        return -1;
    }

    char buf[TIMING_TEXT_MIN_BUFFERSIZE];
    int r2 = timing_string_from_timing(buf, tm);
    if (r2 < 0) {
        perror("timing_string_from_timing");
        return -1;
    }

    printf("%s\n", buf);

    return 0;
}