#ifndef SATURND_CREATE_TASK_H
#define SATURND_CREATE_TASK_H

#include "operations.h"

int handle_taskid(char *path);
int handle_create_task(char *buf, int nbtasks);
int handle_timing(char *path);
int handle_command(char *path);
int handle_runs(char *path);

#endif