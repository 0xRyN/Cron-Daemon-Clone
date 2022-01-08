#include <remove_task.h>
int remove_task(char* REQ_PIPE_PATH, char* RES_PIPE_PATH, uint16_t operation, uint64_t taskid){
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
        close(REQ_FD);
        return -1;
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
        close(RES_FD);
        return -1;
    }
    close(RES_FD);
    return 1;
}