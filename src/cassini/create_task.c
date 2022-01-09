#include "cassini-create-task.h"
int create_task(char* REQ_PIPE_PATH, char* RES_PIPE_PATH, uint16_t operation,int argc,char* argv[],char* minutes_str,char* hours_str,char* daysofweek_str){
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
                close(REQ_FD);
                return -1;
            }

            // Function command_from_args will fill the given struct with the
            // "commandline" type : it will parse "argv" and return a structure
            // of the number of commands and an array of "cstring" for each
            // one. The "command" and the "cstring" structs are custom written.
            if (command_from_args(cmd, argc, argv, optind) == 1) {
                perror("Command_from_args didn't work");
                close(REQ_FD);
                return -1;
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
                close(REQ_FD);
                return -1;
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
                close(RES_FD);
                return -1;
            }
            close(RES_FD);
            return 1;
}