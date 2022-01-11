#ifndef SATURND_GET_STDOUT
#define SATURND_GET_STDOUT

#include "operations.h"

int handle_get_stdout(char *buf);
int stdout_to_cassini(int hasFailed, int taskDoesntExist, int offset);

#endif