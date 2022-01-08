#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int pid = fork();
    if (pid == 0) {
        sleep(1);
    } else
        exit(0);
    return 0;
}