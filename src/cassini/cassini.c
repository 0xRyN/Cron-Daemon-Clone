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

    // Main switch :
    // This is to know which task operation we assigned to Cassini.
    // Each operation will have different requests / responses.
    switch (operation) {
        case CLIENT_REQUEST_LIST_TASKS: {
            if(requestClientRequestTask(REQ_PIPE_PATH, RES_PIPE_PATH, operation)<0){
                goto error;
            }
            break;
        }

        case CLIENT_REQUEST_CREATE_TASK: {
            if(create_task(REQ_PIPE_PATH,RES_PIPE_PATH,operation,argc,argv,minutes_str,hours_str,daysofweek_str) <1){
                goto error;
            }
            break;
        }

        case CLIENT_REQUEST_TERMINATE: {
            if(terminate(REQ_PIPE_PATH, RES_PIPE_PATH, operation)<0){
                goto error;
            }            
            break;
        }

        case CLIENT_REQUEST_REMOVE_TASK: {
            if(remove_task(REQ_PIPE_PATH, RES_PIPE_PATH, operation, taskid)<0){
                goto error;
            }     
            break;
        }

        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES: {
            if(get_time_and_exitcodes(REQ_PIPE_PATH, RES_PIPE_PATH, operation, taskid)<0){
                goto error;
            }   
            break;
        }

        case CLIENT_REQUEST_GET_STDOUT: {
            if(get_stdout(REQ_PIPE_PATH, RES_PIPE_PATH, operation, taskid)<0){
                goto error;
            }   
            break;
        }

        case CLIENT_REQUEST_GET_STDERR: {
            if(get_stderr(REQ_PIPE_PATH, RES_PIPE_PATH, operation, taskid)<0){
                goto error;
            } 
            break;
        }
    }

    // Closing the pipes before exiting.

    // Free the mallocs
    free(REQ_PIPE_PATH);
    free(RES_PIPE_PATH);
    free(pipes_directory);
    pipes_directory = NULL;

    return EXIT_SUCCESS;

error:
    if (errno != 0) perror("main");
    // Closing the pipes before exiting.

    // Free the mallocs
    free(REQ_PIPE_PATH);
    free(RES_PIPE_PATH);
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_FAILURE;
}