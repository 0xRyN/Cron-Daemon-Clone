#include "get_stderr.h"
int get_stderr(char* REQ_PIPE_PATH, char* RES_PIPE_PATH, uint16_t operation, uint64_t taskid){
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
                if (length != 0) {
                    char buf[length];
                    read(RES_FD, buf, length);
                    // Never forget null terminator
                    buf[length] = '\0';
                    printf("%s", buf);
                }

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
                    close(RES_FD);
                    return -1;
                }

                // If the task has not been ran
                else if (err == SERVER_REPLY_ERROR_NEVER_RUN) {
                    printf(
                        "Task with the given taskid wasn't run at least once. "
                        "Exiting...");
                    close(RES_FD);
                    return -1;
                }

                // This shoudln't happen
                else {
                    perror("get_stderr : errcode is corrupted.");
                    close(RES_FD);
                    return -1;
                }

                // Print the error code
                printf("%i", err);
            }

            // This shouldn't happen
            else {
                perror("get_stderr : reptype is corrupted.");
                    close(RES_FD);
                    return -1;
            }
            return 1;
            close(RES_FD);
}