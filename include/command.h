#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>

#include "./cstring.h"

// The "commandline" type. It is called "command" because it is more concise and
// easier to read
struct command {
    uint32_t argc;
    struct cstring* argv;
};

int command_from_args(struct command* dest, int argc, char* argv[], int optind);

#endif  // COMMAND_H
