#include "run-task.h"

// Returns the number of runs already done
int get_cur_run(int taskid) {
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%d/runs", get_username(), taskid);

    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        return -1;
    }
    struct dirent *entry;

    int max = 0;

    while ((entry = readdir(dirp))) {
        // If its not . or ..
        if ((entry->d_name)[0] != '.') {
            int cur = (int)strtol((entry->d_name), NULL, 10);
            if (cur > max) max = cur;
        }
    }

    return max;
}

int dup_stdout(int taskid) {
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%d/stdout", get_username(), taskid);

    int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (fd < 0) {
        perror("Opening stdout");
        return -1;
    }

    int r = dup2(fd, 1);
    if (r < 0) {
        perror("dup2 stdout");
        return -1;
    }

    close(fd);
    return 0;
}

int dup_stderr(int taskid) {
    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%d/stderr", get_username(), taskid);

    int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (fd < 0) {
        perror("Opening stderr");
        return -1;
    }

    int r = dup2(fd, 2);
    if (r < 0) {
        perror("dup2 stderr");
        return -1;
    }

    close(fd);
    return 0;
}

int log_time(int taskid) {
    int cur = get_cur_run(taskid);
    if (cur < 0) {
        perror("Log time - cur runs has failed");
        return -1;
    }
    cur++;

    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%d/runs/%d", get_username(), taskid,
            cur);

    if (access(path, F_OK) != 0) {
        int res_mkdir = mkdir(path, 0777);
        if (res_mkdir < 0) {
            perror("Mkdir log run");
            return -1;
        }
    }

    char path_time[256];
    sprintf(path_time, "/tmp/%s/saturnd/tasks/%d/runs/%d/time", get_username(),
            taskid, cur);

    int fd = open(path_time, O_CREAT | O_WRONLY, 0777);
    if (fd < 0) {
        perror("open log time");
        return -1;
    }

    uint64_t now = time(NULL);

    int bytes_written = write(fd, &now, 8);
    if (bytes_written < 0) {
        perror("Could not write into time");
        return -1;
    }

    close(fd);

    return 0;
}

int log_exitcode(int taskid, int code) {
    int cur = get_cur_run(taskid);
    if (cur < 0) {
        perror("Log exitcode - cur runs has failed");
        return -1;
    }

    char path_exitcode[256];
    sprintf(path_exitcode, "/tmp/%s/saturnd/tasks/%d/runs/%d/exitcode",
            get_username(), taskid, cur);

    int fd = open(path_exitcode, O_CREAT | O_WRONLY, 0777);
    if (fd < 0) {
        perror("open log exitcode");
        return -1;
    }

    uint16_t exitcode;

    if (WIFEXITED(code)) {
        exitcode = WEXITSTATUS(code);
    }

    else
        exitcode = 0xFFFF;

    int bytes_written = write(fd, &exitcode, 2);
    if (bytes_written < 0) {
        perror("Could not write into exitcode");
        return -1;
    }

    close(fd);

    return 0;
}

int run(char **cmd, int taskid) {
    int pid1 = fork();
    if (pid1 == 0) {  // Son
        int pid2 = fork();
        if (pid2 == 0) {  // Son of son
            // Dup on stdout stderr
            int res_stdout = dup_stdout(taskid);
            if (res_stdout < 0) {
                perror("stdout dup");
                return -1;
            }

            int res_stderr = dup_stderr(taskid);
            if (res_stderr < 0) {
                perror("stderr dup");
                return -1;
            }
            int res_log = log_time(taskid);
            if (res_log < 0) {
                perror("Log time");
                return -1;
            }
            execvp(cmd[0], cmd);
        }

        else {  // Son
            int exitcode;
            wait(&exitcode);
            log_exitcode(taskid, exitcode);
            exit(0);
        }
    }
    // Do we want to wait here ? NO
    return 0;
}

int handle_run_task(int tid) {
    // STDOUT - STDERR : ONLY LAST EXECUTION (Open TRUNC)
    // TIMES: EACH EXECUTION, TIME JUST BEFORE EXECUTION
    // EXITCODE : EACH EXECUTION, TIME AFTER FATHER WAIT()

    // IN ORDER NOT TO HANG SATURND, IT WILL HAVE A SON AND ITS SON
    // THE SON WILL WAIT FOR IT'S SON(WHICH WILL RUN THE COMMAND)

    char path[256];
    sprintf(path, "/tmp/%s/saturnd/tasks/%d/command", get_username(), tid);

    struct command *cmd = malloc(sizeof(struct command));
    int r = get_command_from_file(cmd, path);
    if (r < 0) {
        perror("get command from file, here");
        return -1;
    }

    // NEED TO FREE
    char **command_buf = malloc(cmd->argc + 1);
    int i;
    for (i = 0; i < (int)cmd->argc; i++) {
        // NEED TO FREE
        command_buf[i] = malloc(cmd->argv[i].length + 1);
        strncpy(command_buf[i], cmd->argv[i].value, cmd->argv[i].length);
    }
    command_buf[i] = NULL;

    return run(command_buf, tid);
}