#ifndef RUN_TASK_H
#define RUN_TASK_H

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "read-files.h"

int handle_run_task(int taskid);

#endif