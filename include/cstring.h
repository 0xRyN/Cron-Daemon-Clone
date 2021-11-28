#ifndef CSTRING_H
#define CSTRING_H

#include <stdint.h>

// This is the "string" type. It is called "cstring" to avoid confusion from the
// "string.h" header
struct cstring {
    uint32_t length;
    char *value;
};

#endif  // CSTRING_H
