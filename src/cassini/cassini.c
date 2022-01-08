#include "cassini.h"

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

    // Allocate memory for the paths
    char* REQ_PIPE_PATH = malloc(256);
    char* RES_PIPE_PATH = malloc(256);

    // No pipes path given in args
    if (pipes_directory == NULL) {
        // We allocate a buffer for the ABSOLUTE_PATH
        // Default ABSOLUTE_PATH path should be :
        // /tmp/<USER_NAME>/saturnd/pipes
        char* ABSOLUTE_PATH = malloc(256);

        // We allocate a buffer for the user name
        struct passwd* pw;
        unsigned int uid;

        uid = geteuid();
        pw = getpwuid(uid);
        if (!pw) {
            perror("Couldn't get username using getpwuid(uid). Exiting...");
            goto error;
        }

        // Sadly forced to do this...
        ABSOLUTE_PATH = strcat(ABSOLUTE_PATH, "/tmp/");
        ABSOLUTE_PATH = strcat(ABSOLUTE_PATH, pw->pw_name);
        ABSOLUTE_PATH = strcat(ABSOLUTE_PATH, "/saturnd/pipes");

        strcpy(REQ_PIPE_PATH, ABSOLUTE_PATH);
        strcpy(RES_PIPE_PATH, ABSOLUTE_PATH);

        free(ABSOLUTE_PATH);

        REQ_PIPE_PATH = strcat(REQ_PIPE_PATH, REQ_PIPE);
        RES_PIPE_PATH = strcat(RES_PIPE_PATH, RES_PIPE);
    }

    // Else, if pipes path are given in args :
    else {
        strcpy(REQ_PIPE_PATH, pipes_directory);
        strcpy(RES_PIPE_PATH, pipes_directory);
        REQ_PIPE_PATH = strcat(REQ_PIPE_PATH, REQ_PIPE);
        RES_PIPE_PATH = strcat(RES_PIPE_PATH, RES_PIPE);
    }

    // Open the two pipes, request and response, in WRITEONLY and READONLY
    // respectively
    //int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
    int RES_FD = open(RES_PIPE_PATH, O_RDONLY);

    // We check that there are no errors with the pipes
    

    if (RES_FD == -1) {
        perror("Error when opening response pipe");
        goto error;
    }

    // Main switch :
    // This is to know which task operation we assigned to Cassini.
    // Each operation will have different requests / responses.
    switch (operation) {
        case CLIENT_REQUEST_LIST_TASKS: {
            // Convert and write operation
            int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
            uint16_t op = htobe16(operation);
            if (write(REQ_FD, &op, 2) < 2) {
                perror("LIST_TASKS : Write error...");
                goto error;
            }
            close(REQ_FD);

            int RES_FD = open(RES_PIPE_PATH, O_RDONLY);

            // We sent the operation to the daemon. We will now read the
            // response...
            uint16_t reptype;
            read(RES_FD, &reptype, 2);

            // Checking if the daemon response is OK...
            if (be16toh(reptype) == SERVER_REPLY_ERROR) {
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
            close(RES_FD);
            break;
        }

        case CLIENT_REQUEST_CREATE_TASK: {
            // Timing pointer for timing_from_strings
            int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
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

            // Buffer size = op (2) + timing (8 + 4 + 1) + command_argc (4)
            int buf_size = 0;
            buf_size = 2 + 8 + 4 + 1 + 4;

            // For each command, add the length (4) and the value of the length
            uint32_t command_argc = cmd->argc;
            for (unsigned int i = 0; i < cmd->argc; i++) {
                buf_size += 4;
                buf_size += cmd->argv[i].length;
            }

            // Assign the write buffer to it's exact size
            char buf[buf_size];

            // Add an offset variable for memcpy
            int offset = 0;

            // Convert and add to buffer, update offset
            u_int16_t op = htobe16(operation);
            memcpy(buf + offset, &(op), 2);
            offset += 2;

            // BEGIN TIMING - Convert and add to buffer, update offset

            time->hours = htobe32(time->hours);
            time->minutes = htobe64(time->minutes);
            memcpy(buf + offset, &(time->minutes), 8);
            offset += 8;
            memcpy(buf + offset, &(time->hours), 4);
            offset += 4;
            memcpy(buf + offset, &(time->daysofweek), 1);
            offset += 1;

            // END TIMING

            // BEGIN COMMANDLINE : Convert and add to buffer, argc and argv

            // ARGC
            // Convert and add to buffer, update offset
            command_argc = htobe32(command_argc);
            memcpy(buf + offset, &command_argc, 4);
            offset += 4;

            // ARGV
            // For each command, convert and add to buffer, update offset
            for (unsigned int i = 0; i < cmd->argc; i++) {
                uint32_t converted_length = htobe32(cmd->argv[i].length);
                uint32_t length = cmd->argv[i].length;

                memcpy(buf + offset, &(converted_length), 4);
                offset += 4;
                memcpy(buf + offset, cmd->argv[i].value, length);
                offset += length;
            }

            // END COMMANDLINE

            // Giant satisfying write. Writes everything to request fifo
            if (write(REQ_FD, buf, buf_size) < buf_size) {
                perror("CREATE_TASK : Write error...");
                goto error;
            }
            close(REQ_FD);

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
            int RES_FD = open(RES_PIPE_PATH, O_RDONLY);
            uint16_t reptype;
            read(RES_FD, &reptype, 2);

            // If the response is "OK" -> "0x4f4b", we read the task id
            // assigned for this particular task and print it
            if (be16toh(reptype) == SERVER_REPLY_OK) {
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
            close(RES_FD);
            break;
        }

        case CLIENT_REQUEST_TERMINATE: {
            int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
            // Convert and write operation
            uint16_t op = htobe16(operation);
            if (write(REQ_FD, &op, 2) < 2) {
                perror("REQUEST_TERMINATE : Write error...");
                goto error;
            }
            close(REQ_FD);
            // Wait for the response
            int RES_FD = open(RES_PIPE_PATH, O_RDONLY);
            uint16_t reptype;
            read(RES_FD, &reptype, 2);

            // Checking if the daemon response is OK...
            if (be16toh(reptype) == SERVER_REPLY_ERROR) {
                perror(
                    " CLIENT_REQUEST_REMOVE_TASK: : Daemon responded with an "
                    "error code, exiting...");
                goto error;
            }
            close(RES_FD);
            break;
        }

        case CLIENT_REQUEST_REMOVE_TASK: {
            int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
            uint16_t op = htobe16(operation);  // Operation for the request
            uint64_t tId = htobe64(taskid);    // Task id for the the request

            // Allocate a buffer of size operation + taskid
            int size = sizeof(op) + sizeof(taskid);
            char buf[size];

            // Move all data to the buffer
            memmove(buf, &op, sizeof(op));
            memmove(buf + sizeof(op), &tId, sizeof(tId));

            // Write the buffer to the request pipe
            if (write(REQ_FD, buf, size) < size) {
                perror("REMOVE_TASK : Write error...");
                goto error;
            }
            close(REQ_FD);

            // Wait for the response
            int RES_FD = open(RES_PIPE_PATH, O_RDONLY);
            uint16_t reptype;
            read(RES_FD, &reptype, 2);

            // Checking if the daemon response is OK...
            if (be16toh(reptype) == SERVER_REPLY_ERROR) {
                perror(
                    " CLIENT_REQUEST_REMOVE_TASK: : Daemon responded with an "
                    "error code, exiting...");
                goto error;
            }
            close(RES_FD);
            break;
        }

        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES: {
            int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
            uint16_t op = htobe16(operation);  // Operation for the request
            uint64_t tId = htobe64(taskid);    // Task id for the the request

            // Allocate a buffer of size operation + taskid
            int size = sizeof(op) + sizeof(taskid);
            char buf[size];

            // Move all data to the buffer
            memmove(buf, &op, sizeof(op));
            memmove(buf + sizeof(op), &tId, sizeof(tId));

            // Write the buffer to the request pipe
            if (write(REQ_FD, buf, size) < size) {
                perror("GET_TIMES : Write error...");
                goto error;
            }
            close(REQ_FD);

            // Wait for the response
            int RES_FD = open(RES_PIPE_PATH, O_RDONLY);
            uint16_t reptype;
            read(RES_FD, &reptype, 2);

            // Checking if the daemon response is OK...
            if (be16toh(reptype) == SERVER_REPLY_ERROR) {
                perror(
                    " CLIENT_REQUEST_REMOVE_TASK: : Daemon responded with an "
                    "error code, exiting...");
                goto error;
            }

            // Read number of runs
            uint32_t nbRuns;
            read(RES_FD, &nbRuns, 4);
            nbRuns = be32toh(nbRuns);

            // For each run, read the data and print it
            for (unsigned int i = 0; i < nbRuns; i++) {
                // Read and convert time
                int64_t run_time;
                read(RES_FD, &run_time, 8);
                run_time = be64toh(run_time);

                // Build a struct from the run's time
                struct tm* timeInfos = localtime(&run_time);

                // Read and convert exitcode
                uint16_t exitcode;
                read(RES_FD, &exitcode, 2);
                exitcode = be16toh(exitcode);

                // Do the necessary print
                // PS : Mon is increased by one because first value is 0
                printf("%04d-%02d-%02d %02d:%02d:%02d %i\n",
                       timeInfos->tm_year + 1900, timeInfos->tm_mon + 1,
                       timeInfos->tm_mday, timeInfos->tm_hour,
                       timeInfos->tm_min, timeInfos->tm_sec, exitcode);
            }
            close(RES_FD);
            break;
        }

        case CLIENT_REQUEST_GET_STDOUT: {
            int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
            uint16_t op = htobe16(operation);  // Operation for the request
            uint64_t tId = htobe64(taskid);    // Task id for the the request

            // Allocate a buffer of size operation + taskid
            int size = sizeof(op) + sizeof(taskid);
            char buf[size];

            // Move all data to the buffer
            memmove(buf, &op, sizeof(op));
            memmove(buf + sizeof(op), &tId, sizeof(tId));

            // Write the buffer to the request pipe
            if (write(REQ_FD, buf, size) < size) {
                perror("GET_TIMES : Write error...");
                goto error;
            }
            close(REQ_FD);

            // Read and convert the response
            int RES_FD = open(RES_PIPE_PATH, O_RDONLY);
            uint16_t reptype;
            read(RES_FD, &reptype, 2);
            reptype = be16toh(reptype);

            // Daemon responded successfully
            if (reptype == SERVER_REPLY_OK) {
                // Read and convert the length of the string
                uint32_t length;
                read(RES_FD, &length, sizeof(length));
                length = be32toh(length);

                // Alloc a buffer, read length bytes of memory to it and print
                char buf[length];
                read(RES_FD, buf, length - 1);
                printf("%s", buf);
            }

            // Daemon had an error
            else if (reptype == SERVER_REPLY_ERROR) {
                // Read and convert the error code
                uint16_t err;
                read(RES_FD, &err, 2);
                err = be16toh(err);

                // If the task has not been found
                if (err == SERVER_REPLY_ERROR_NOT_FOUND) {
                    printf(
                        "Task with the given taskid was not found. Exiting...");
                    goto error;
                }

                // If the task has not been ran
                else if (err == SERVER_REPLY_ERROR_NEVER_RUN) {
                    printf(
                        "Task with the given taskid wasn't run at least once. "
                        "Exiting...");
                    goto error;
                }

                // This shoudln't happen
                else {
                    perror("get_stdout : errcode is corrupted.");
                    goto error;
                }

                // Print the error code
                printf("%i", err);
            }

            // This shouldn't happen
            else {
                perror("get_stdout : reptype is corrupted.");
                goto error;
            }
            close(RES_FD);
            break;
        }

        case CLIENT_REQUEST_GET_STDERR: {
            int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
            uint16_t op = htobe16(operation);  // Operation for the request
            uint64_t tId = htobe64(taskid);    // Task id for the the request

            // Allocate a buffer of size operation + taskid
            int size = sizeof(op) + sizeof(taskid);
            char buf[size];

            // Move all data to the buffer
            memmove(buf, &op, sizeof(op));
            memmove(buf + sizeof(op), &tId, sizeof(tId));

            // Write the buffer to the request pipe
            if (write(REQ_FD, buf, size) < size) {
                perror("GET_TIMES : Write error...");
                goto error;
            }
            close(REQ_FD);

            // Read and convert the response
            int RES_FD = open(RES_PIPE_PATH, O_RDONLY);
            uint16_t reptype;
            read(RES_FD, &reptype, 2);
            reptype = be16toh(reptype);

            // Daemon responded successfully
            if (reptype == SERVER_REPLY_OK) {
                // Read and convert the length of the string
                uint32_t length;
                read(RES_FD, &length, sizeof(length));
                length = be32toh(length);

                // Alloc a buffer, read length bytes of memory to it and print
                char buf[length];
                read(RES_FD, buf, length - 1);
                printf("%s", buf);
            }

            // Daemon had an error
            else if (reptype == SERVER_REPLY_ERROR) {
                // Read and convert the error code
                uint16_t err;
                read(RES_FD, &err, 2);
                err = be16toh(err);

                // If the task has not been found
                if (err == SERVER_REPLY_ERROR_NOT_FOUND) {
                    printf(
                        "Task with the given taskid was not found. Exiting...");
                    goto error;
                }

                // If the task has not been ran
                else if (err == SERVER_REPLY_ERROR_NEVER_RUN) {
                    printf(
                        "Task with the given taskid wasn't run at least once. "
                        "Exiting...");
                    goto error;
                }

                // This shoudln't happen
                else {
                    perror("get_stderr : errcode is corrupted.");
                    goto error;
                }

                // Print the error code
                printf("%i", err);
            }

            // This shouldn't happen
            else {
                perror("get_stderr : reptype is corrupted.");
                goto error;
            }
            close(RES_FD);
            break;
        }
    }

    // Closing the pipes before exiting.
    //close(REQ_FD);
    //close(RES_FD);

    // Free the mallocs
    free(REQ_PIPE_PATH);
    free(RES_PIPE_PATH);
    free(pipes_directory);
    pipes_directory = NULL;

    return EXIT_SUCCESS;

error:
    if (errno != 0) perror("main");
    // Closing the pipes before exiting.
    //close(REQ_FD);
    //close(RES_FD);

    // Free the mallocs
    free(REQ_PIPE_PATH);
    free(RES_PIPE_PATH);
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_FAILURE;
}