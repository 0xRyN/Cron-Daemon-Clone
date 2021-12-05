#ifndef CASSINI_H
#define CASSINI_H

#define _DEFAULT_SOURCE

#include <cstring.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"
#include "time.h"
#include "timing-text-io.h"
#include "timing.h"
#include "util.h"

#define REQ_PIPE "/saturnd-request-pipe"
#define RES_PIPE "/saturnd-reply-pipe"

#include "client-request.h"
#include "server-reply.h"

#endif  // CASSINI
