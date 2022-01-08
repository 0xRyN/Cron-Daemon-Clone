#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <string.h>

// Just an utilitary header for utilitary functions

struct pipes_paths {
    char *REQ_PATH;
    char *RES_PATH;
};

rev(char *s);

#endif  // UTIL_H