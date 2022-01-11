#ifndef SATURND_GET_TIME_AND_EXITCODES_H
#define SATURND_GET_TIME_AND_EXITCODES_H

#include "operations.h"

int time_exitcode_to_cassini(int hasFailed, int offset);
int iterate(int taskid, int off);
int get_nbruns(int taskId);
int handle_get_time_exitcode(char *b);

#endif