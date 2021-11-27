#define _DEFAULT_SOURCE

#include "command.h"

#include <cstring.h>
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

// Builds a command structure. Writes the struct into *dest. Returns 0 if
// everything is fine. Else, will return 1;
int command_from_args(struct command* dest, int argc, char* argv[],
                      int optind) {
    int k = 0;
    int num_commands = argc - optind;
    dest->argc = num_commands;
    dest->argv = malloc(sizeof(struct cstring) * num_commands);
    for (int i = optind; i < argc; i++) {
        char* cur = argv[i];
        int32_t len = strlen(cur);
        struct cstring str;
        str.value = malloc(len);
        strcpy(str.value, argv[i]);

        str.length = htobe32(len);
        dest->argv[k] = str;
        k++;
    }
    return 0;
}