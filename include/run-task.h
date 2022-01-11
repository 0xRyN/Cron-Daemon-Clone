#ifndef RUN_TASK_H
#define RUN_TASK_H

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "read-files.h"
#include "util.h"

int handle_run_task(int taskid);
int get_cur_run(int taskid);
int dup_stdout(int taskid);
int dup_stderr(int taskid);
int log_time(int taskid);
int log_exitcode(int taskid, int code);
int run(char **cmd, int taskid);

#endif