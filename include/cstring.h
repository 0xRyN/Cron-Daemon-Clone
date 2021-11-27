#ifndef CSTRING_H
#define CSTRING_H

#include <stdint.h>

struct cstring {
    uint32_t length;
    char *value;
};

#endif  // CSTRING_H
