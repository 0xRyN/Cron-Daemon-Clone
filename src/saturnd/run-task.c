#include "run-task.h"

int handle_run_task(int taskid) {
    // STDOUT - STDERR : ONLY LAST EXECUTION (Open TRUNC)
    // TIMES: EACH EXECUTION, TIME JUST BEFORE EXECUTION
    // EXITCODE : EACH EXECUTION, TIME AFTER FATHER WAIT()

    // IN ORDER NOT TO HANG SATURND, IT WILL HAVE A SON AND ITS SON
    // THE SON WILL WAIT FOR IT'S SON(WHICH WILL RUN THE COMMAND)

    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%d/command", get_username(), taskid);
}