#include "cassini-terminate.h"
int terminate(char* REQ_PIPE_PATH, char* RES_PIPE_PATH, uint16_t operation){
    int REQ_FD = open(REQ_PIPE_PATH, O_WRONLY);
        // Convert and write operation
            uint16_t op = htobe16(operation);
            if (write(REQ_FD, &op, 2) < 2) {
                perror("REQUEST_TERMINATE : Write error...");
                close(REQ_FD);
                return -1;
                return 1;
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
            return 1;
            close(RES_FD);
}