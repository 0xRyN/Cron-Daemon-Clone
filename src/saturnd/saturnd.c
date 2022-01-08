#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int pid = fork();
    if (pid != 0) exit(0);
    return 0;
}