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

    // struct command cmd;
    // if (command_from_args(&cmd, argc, argv, optind) == 1) {
    //     perror("Command_from_args didn't work");
    //     goto error;
    // };
    // printf("Struct contains %i commands\n", cmd.argc);
    // printf("First one is : %i\n", cmd.argv[1].length);

    // --------
    // | TODO |
    // --------

    int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
    int RES_FD = open(RES_PIPE_PATH, O_RDONLY);
    if (REQ_FD == -1) {
        perror("Error when opening request pipe");
        goto error;
    }

    if (RES_FD == -1) {
        perror("Error when opening response pipe");
        goto error;
    }
    uint16_t op = htobe16(operation);
    int w = write(REQ_FD, &op, sizeof(uint16_t));
    if (w == -1) {
        perror("Error when writing to request pipe");
        goto error;
    }

    switch (operation) {
        case CLIENT_REQUEST_LIST_TASKS:
            break;

        case CLIENT_REQUEST_CREATE_TASK: {
            struct timing* time = malloc(sizeof(struct timing));
            struct command* cmd = malloc(sizeof(struct command));
            if (command_from_args(cmd, argc, argv, optind) == 1) {
                perror("Command_from_args didn't work");
                goto error;
            };
            if (timing_from_strings(time, minutes_str, hours_str,
                                    daysofweek_str) == -1) {
                perror("Could not use timing_from_strings");
                goto error;
            }
            time->hours = htobe32(time->hours);
            time->minutes = htobe64(time->minutes);
            write(REQ_FD, &(time->minutes), 8);
            write(REQ_FD, &(time->hours), 4);
            write(REQ_FD, &(time->daysofweek), 1);

            int ac = htobe32(cmd->argc);
            write(REQ_FD, &ac, 4);
            for (unsigned int i = 0; i < cmd->argc; i++) {
                write(REQ_FD, &(cmd->argv[i].length), 4);
                write(REQ_FD, cmd->argv[i].value, strlen(cmd->argv[i].value));
            }

            // BEGIN - FREEING ALL POINTERS
            for (unsigned int i = 0; i < cmd->argc; i++) {
                free(cmd->argv[i].value);
            }
            free(cmd->argv);
            free(cmd);
            free(time);
            // END - FREEING ALL POINTERS

            uint16_t reptype;

            read(RES_FD, &reptype, 4);
            if (htobe16(reptype) == 0x4f4b)
                printf("0");
            else {
                printf("1");
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
    close(REQ_FD);
    close(RES_FD);
    return EXIT_SUCCESS;

error:
    if (errno != 0) perror("main");
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_FAILURE;
}
