#ifndef UTIL_H
#define UTIL_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Just an utilitary header for utilitary functions

struct pipes_paths {
    char *ABS_PATH;
    char *REQ_PATH;
    char *RES_PATH;
};

char *rev(char *s);

struct pipes_paths *get_default_paths();

void _mkdir(const char *dir);

int _rmdir(const char *path);

char *get_username();

#endif  // UTIL_H