#define _DEFAULT_SOURCE

#include "cassini.h"

#include <endian.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"
#include "timing-text-io.h"
#include "timing.h"
#include "util.h"

#define REQ_PIPE_PATH "./run/pipes/saturnd-request-pipe"
#define RES_PIPE_PATH "./run/pipes/saturnd-reply-pipe"

// list of cassini options and commands
const char usage_info[] =
    "\
   usage: cassini [OPTIONS] -l -> list all tasks\n\
      or: cassini [OPTIONS]    -> same\n\
      or: cassini [OPTIONS] -q -> terminate the daemon\n\
      or: cassini [OPTIONS] -c [-m MINUTES] [-H HOURS] [-d DAYSOFWEEK] COMMAND_NAME [ARG_1] ... [ARG_N]\n\
          -> add a new task and print its TASKID\n\
             format & semantics of the \"timing\" fields defined here:\n\
             https://pubs.opengroup.org/onlinepubs/9699919799/utilities/crontab.html\n\
             default value for each field is \"*\"\n\
      or: cassini [OPTIONS] -r TASKID -> remove a task\n\
      or: cassini [OPTIONS] -x TASKID -> get info (time + exit code) on all the past runs of a task\n\
      or: cassini [OPTIONS] -o TASKID -> get the standard output of the last run of a task\n\
      or: cassini [OPTIONS] -e TASKID -> get the standard error\n\
      or: cassini -h -> display this message\n\
\n\
   options:\n\
     -p PIPES_DIR -> look for the pipes in PIPES_DIR (default: /tmp/<USERNAME>/saturnd/pipes)\n\
";

int main(int argc, char* argv[]) {
    errno = 0;

    char* minutes_str = "*";
    char* hours_str = "*";
    char* daysofweek_str = "*";
    char* pipes_directory = NULL;

    uint16_t operation = CLIENT_REQUEST_LIST_TASKS;
    uint64_t taskid;

    int opt;
    char* strtoull_endp;
    // Parsing command line arguments...
    while ((opt = getopt(argc, argv, "hlcqm:H:d:p:r:x:o:e:")) != -1) {
        switch (opt) {
            case 'm':
                minutes_str = optarg;
                break;
            case 'H':
                hours_str = optarg;
                break;
            case 'd':
                daysofweek_str = optarg;
                break;
            case 'p':
                pipes_directory = strdup(optarg);
                if (pipes_directory == NULL) goto error;
                break;
            case 'l':
                operation = CLIENT_REQUEST_LIST_TASKS;
                break;
            case 'c':
                operation = CLIENT_REQUEST_CREATE_TASK;
                break;
            case 'q':
                operation = CLIENT_REQUEST_TERMINATE;
                break;
            case 'r':
                operation = CLIENT_REQUEST_REMOVE_TASK;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0')
                    goto error;
                break;
            case 'x':
                operation = CLIENT_REQUEST_GET_TIMES_AND_EXITCODES;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0')
                    goto error;
                break;
            case 'o':
                operation = CLIENT_REQUEST_GET_STDOUT;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0')
                    goto error;
                break;
            case 'e':
                operation = CLIENT_REQUEST_GET_STDERR;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0')
                    goto error;
                break;
            case 'h':
                printf("%s", usage_info);
                return 0;
            case '?':
                fprintf(stderr, "%s", usage_info);
                goto error;
        }
    }

    // --------
    // | TODO |
    // --------

    // Open the two pipes, request and response, in WRITEONLY and READONLY
    // respectively
    int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
    int RES_FD = open(RES_PIPE_PATH, O_RDONLY);

    // We will always send an operation to request. This is why
    // it is out of the switch
    uint16_t op = htobe16(operation);
    int w = write(REQ_FD, &op, sizeof(uint16_t));
    if (w == -1) {
        perror("Error when writing to request pipe");
        goto error;
    }

    // We check that there are no errors with the pipes
    if (REQ_FD == -1) {
        perror("Error when opening request pipe");
        goto error;
    }

    if (RES_FD == -1) {
        perror("Error when opening response pipe");
        goto error;
    }

    // Main switch :
    // This is to know which task operation we assigned to Cassini.
    // Each operation will have different requests / responses.
    switch (operation) {
        case CLIENT_REQUEST_LIST_TASKS: {
            // We sent the operation to the daemon. We will now read the
            // response...
            uint16_t reptype;
            read(RES_FD, &reptype, 2);

            // Checking if the daemon response is OK...
            if (be16toh(reptype) == 0x4552) {
                perror(
                    "CLIENT_REQUEST_LIST_TASKS : Daemon responded with an "
                    "error code, exiting...");
                goto error;
            }

            uint32_t nbtasks;       // Number of tasks
            uint64_t res_taskid;    // Task id
            uint64_t minutes;       // Timing.minutes
            uint32_t hours;         // Timing.hours
            uint8_t day;            // Timing.daysOfWeek
            uint32_t command_argc;  // Command.argc

            read(RES_FD, &nbtasks, sizeof(nbtasks));
            nbtasks = be32toh(nbtasks);

            // If tasks exists, display them in the protocol's format
            if (nbtasks > 0) {
                for (uint32_t i = 0; i < nbtasks; i++) {
                    // Allocate a struct for timing
                    struct timing* time = malloc(13);

                    // Read and print the taskid
                    read(RES_FD, &res_taskid, 8);
                    printf("%li: ", be64toh(res_taskid));

                    // Read and print the minutes
                    read(RES_FD, &minutes, 8);

                    // Read and print the hours
                    read(RES_FD, &hours, 4);

                    // Read and print the days
                    read(RES_FD, &day, 1);

                    // Perform the necessary conversions
                    time->minutes = be64toh(minutes);
                    time->hours = be32toh(hours);
                    time->daysofweek = day;

                    // Transform the timing to a string
                    char str[TIMING_TEXT_MIN_BUFFERSIZE];
                    timing_string_from_timing(str, time);
                    printf("%s ", str);

                    // Read the number of commands in "commandline" type
                    read(RES_FD, &command_argc, 4);
                    command_argc = be32toh(command_argc);

                    // For each command, display it's info
                    for (unsigned int i = 0; i < command_argc; i++) {
                        // First, we read the length of each string
                        int str_length;
                        read(RES_FD, &str_length, 4);
                        str_length = be32toh(str_length);

                        // Then, we allocate a buffer of "str_length" bits and
                        // read "str_length" bits into the buffer.
                        // Then, we will also print it.
                        char* str_data = malloc(str_length);
                        read(RES_FD, str_data, str_length);
                        printf("%s ", str_data);

                        // Finally, we can free the buffer
                        free(str_data);
                    }
                    // After each task, return to line
                    printf("\n");
                }
            }
            break;
        }

        case CLIENT_REQUEST_CREATE_TASK: {
            // Timing pointer for timing_from_strings
            struct timing* time = malloc(sizeof(struct timing));

            // Command pointer for command_from_args
            struct command* cmd = malloc(sizeof(struct command));

            // Function timing_from_strings will parse the strings given as
            // argument and return a timing struct.
            if (timing_from_strings(time, minutes_str, hours_str,
                                    daysofweek_str) == -1) {
                perror("Could not use timing_from_strings");
                goto error;
            }

            // Function command_from_args will fill the given struct with the
            // "commandline" type : it will parse "argv" and return a structure
            // of the number of commands and an array of "cstring" for each
            // one. The "command" and the "cstring" structs are custom written.
            if (command_from_args(cmd, argc, argv, optind) == 1) {
                perror("Command_from_args didn't work");
                goto error;
            };

            // BEGIN TIMING : convert and write the timing to the pipe
            time->hours = htobe32(time->hours);
            time->minutes = htobe64(time->minutes);
            write(REQ_FD, &(time->minutes), 8);
            write(REQ_FD, &(time->hours), 4);
            write(REQ_FD, &(time->daysofweek), 1);

            // END TIMING

            // BEGIN COMMANDLINE : convert and write the commandline to the pipe

            // There is command_argc commands to write.
            int command_argc = htobe32(cmd->argc);
            write(REQ_FD, &command_argc, 4);

            // For each command, write it and it's length to the pipe.
            for (unsigned int i = 0; i < cmd->argc; i++) {
                // Write the length of the command
                write(REQ_FD, &(cmd->argv[i].length), 4);

                // Write the value of the command (string)
                write(REQ_FD, cmd->argv[i].value, strlen(cmd->argv[i].value));
            }

            // END COMMANDLINE

            // BEGIN - FREEING ALL POINTERS
            for (unsigned int i = 0; i < cmd->argc; i++) {
                free(cmd->argv[i].value);
            }
            free(cmd->argv);
            free(cmd);
            free(time);
            // END - FREEING ALL POINTERS

            // We now sent all the requests needed, we now wait for
            // a response from the daemon.

            // Reading the daemon's response from the response pipe
            uint16_t reptype;
            read(RES_FD, &reptype, 2);

            // If the response is "OK" -> "0x4f4b", we read the task id
            // assigned for this particular task and print it
            if (be16toh(reptype) == 0x4f4b) {
                uint64_t res_taskid;
                read(RES_FD, &res_taskid, 8);
                printf("%lu", be64toh(res_taskid));
            }

            // If the response is not "OK", we display an error message and exit
            // the program cleanly by calling error:
            else {
                perror(
                    "CLIENT_REQUEST_CREATE_TASK : Daemon responded with an "
                    "error code, exiting...");
                goto error;
            }
            break;
        }

        case CLIENT_REQUEST_TERMINATE:
            break;

        case CLIENT_REQUEST_REMOVE_TASK:
            break;

        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES:
            break;

        case CLIENT_REQUEST_GET_STDOUT:
            break;

        case CLIENT_REQUEST_GET_STDERR:
            break;
    }

    // Closing the pipes before exiting.
    close(REQ_FD);
    close(RES_FD);

    // Freeing pipes directory before exiting.
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_SUCCESS;

error:
    if (errno != 0) perror("main");
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_FAILURE;
}
