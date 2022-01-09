#ifndef SATURND_CREATE_TASK_H
#define SATURND_CREATE_TASK_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "client-request.h"
#include "command.h"
#include "cstring.h"
#include "operations.h"
#include "server-reply.h"
#include "timing-text-io.h"
#include "timing.h"
#include "util.h"

int handle_taskid(char *path);
int handle_create_task(char *buf, int nbtasks);

#endif