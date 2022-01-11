#ifndef SATURND_H
#define SATURND_H

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "daemon.h"
#include "global.h"
#include "operations.h"
#include "pollfd.h"
#include "run-task.h"
#include "saturnd-check-tasks.h"
#include "util.h"

#endif