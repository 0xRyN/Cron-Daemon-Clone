#ifndef SATURND_GET_STDERR
#define SATURND_GET_STDERR

#include "operations.h"

int handle_get_stderr(char *buf);
int stderr_to_cassini(int hasFailed, int taskDoesntExist, int offset);

#endif