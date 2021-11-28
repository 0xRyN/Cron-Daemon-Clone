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
    // Initialize "dest" struc. Allocate memory equal to the size of a "cstring"
    // times the number of non-option arguments (checked by optind).
    // The command struct "dest" will contain the number of commands as the
    // "argc" variable
    int k = 0;
    int num_commands = argc - optind;
    dest->argc = num_commands;
    dest->argv = malloc(sizeof(struct cstring) * num_commands);

    // Malloc failed
    if (dest->argv == NULL) {
        perror("In command.c \"malloc(dest->argv)\": malloc failed...");
        return 1;
    }

    // For each non-option argv string (argv[optind] to argv[argc-1]), create a
    // new cstring with it's length and data, and then put the cstring into the
    // command struct "dest".
    for (int i = optind; i < argc; i++) {
        // Create the cstring struct
        struct cstring str;

        // Set the length of the "cstring"
        int32_t cstring_length = strlen(argv[i]);
        str.length = htobe32(cstring_length);

        // Set the value of the "cstring"
        str.value = malloc(cstring_length);
        // Malloc failed
        if (str.value == NULL) {
            perror("In command.c \"malloc(str.value)\": malloc failed...");
            return 1;
        }

        strcpy(str.value, argv[i]);

        // Finally, add the string to the "command" structure
        dest->argv[k] = str;
        k++;
    }
    // Everything is fine. Return 0
    return 0;

    // Free the dangling pointers after usage.
}