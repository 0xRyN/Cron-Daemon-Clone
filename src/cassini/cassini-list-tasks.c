#include "cassini-list-tasks.h"

int requestClientRequestTask(char* REQ_PIPE_PATH, char* RES_PIPE_PATH,
                             uint16_t operation) {
    // Convert and write operation
    int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
    uint16_t op = htobe16(operation);
    if (write(REQ_FD, &op, 2) < 2) {
        perror("LIST_TASKS : Write error...");
        close(REQ_FD);
        return -1;
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
        close(RES_FD);
        return -1;
    }

    uint32_t nbtasks;       // Number of tasks
    uint64_t res_taskid;    // Task id
    uint64_t minutes;       // Timing.minutes
    uint32_t hours;         // Timing.hours
    uint8_t day;            // Timing.daysOfWeek
    uint32_t command_argc;  // Command.argc

    read(RES_FD, &nbtasks, sizeof(nbtasks));
    printf("%d\n", nbtasks);
    nbtasks = be32toh(nbtasks);
    printf("%d\n", nbtasks);

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
    return 1;
}