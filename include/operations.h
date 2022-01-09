#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "client-request.h"
#include "command.h"
#include "create-task.h"
#include "cstring.h"
#include "server-reply.h"
#include "timing-text-io.h"
#include "util.h"

int handle_operation(uint16_t opcode, int req_fd);

#endif