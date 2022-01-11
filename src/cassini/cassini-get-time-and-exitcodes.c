#include "cassini-get-time-and-exitcodes.h"

int get_time_and_exitcodes(char* REQ_PIPE_PATH, char* RES_PIPE_PATH,
                           uint16_t operation, uint64_t taskid) {
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
        close(REQ_FD);
        return -1;
    }
    close(REQ_FD);

    // Wait for the response
    int RES_FD = open(RES_PIPE_PATH, O_RDONLY);
    uint16_t reptype;
    read(RES_FD, &reptype, 2);

    reptype = be16toh(reptype);
    // Checking if the daemon response is OK...
    if (reptype == SERVER_REPLY_ERROR) {
        perror(
            " CLIENT_REQUEST_REMOVE_TASK: : Daemon responded with an "
            "error code, exiting...");
        close(RES_FD);
        return -1;
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
        printf("%04d-%02d-%02d %02d:%02d:%02d %i\n", timeInfos->tm_year + 1900,
               timeInfos->tm_mon + 1, timeInfos->tm_mday, timeInfos->tm_hour,
               timeInfos->tm_min, timeInfos->tm_sec, exitcode);

        // Alternative beautiful print
        // printf("Task id: %i %04d-%02d-%02d %02d:%02d:%02d %i\n", i,
        //        timeInfos->tm_year + 1900, timeInfos->tm_mon + 1,
        //        timeInfos->tm_mday, timeInfos->tm_hour, timeInfos->tm_min,
        //        timeInfos->tm_sec, exitcode);
    }
    close(RES_FD);
    return 1;
}