#ifndef SATURND_CHECK_TASKS
#define SATURND_CHECK_TASKS

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "read-files.h"
#include "run-task.h"
#include "timing-text-io.h"

int handle_check_tasks();

#endif