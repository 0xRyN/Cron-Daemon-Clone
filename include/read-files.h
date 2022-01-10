#ifndef READ_FILES_H
#define READ_FILES_H

#undef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE

#include <cstring.h>
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "cstring.h"
#include "timing.h"

int get_timing_from_file(struct timing *time, char *path);
int get_taskid_from_file(uint64_t *tId, char *path);
int get_command_from_file(struct command *cmd, char *path);

#endif