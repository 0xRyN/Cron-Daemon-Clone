#include "util.h"

/* /!\ NOT USED FOR CASSINI - BUT COULD BE USED FOR THE DAEMON /!\ */

// Simple function to reverse a string. Returns the reversed string.
char *rev(char *s) {
    // Assign two pointers, one at the beginning, one at the end
    char *ptr1 = s;
    char *ptr2 = s + strlen(s) - 1;  // One char before '\0'

    // While ptr2 > ptr1, swap the values of both pointers, increment p1 and
    // decrement p2. This way, null terminator '\0' stays in place
    while (ptr2 > ptr1) {
        char t = *ptr1;
        *ptr1 = *ptr2;
        *ptr2 = t;
        ptr2--;
        ptr1++;
    }
    return s;
}

struct pipes_paths *get_default_paths() {
    struct pipes_paths *paths = malloc(sizeof(struct pipes_paths));
    paths->REQ_PATH = malloc(256);
    paths->RES_PATH = malloc(256);
    paths->ABS_PATH = malloc(256);

    char *ABSOLUTE_PATH = malloc(256);

    // Sadly forced to do this...
    ABSOLUTE_PATH = strcat(ABSOLUTE_PATH, "/tmp/");
    ABSOLUTE_PATH = strcat(ABSOLUTE_PATH, getenv("USER"));
    ABSOLUTE_PATH = strcat(ABSOLUTE_PATH, "/saturnd/pipes");

    strcpy(paths->REQ_PATH, ABSOLUTE_PATH);
    strcpy(paths->RES_PATH, ABSOLUTE_PATH);

    strcpy(paths->ABS_PATH, ABSOLUTE_PATH);
    paths->REQ_PATH = strcat(paths->REQ_PATH, "/saturnd-request-pipe");
    paths->RES_PATH = strcat(paths->RES_PATH, "/saturnd-reply-pipe");
    free(ABSOLUTE_PATH);
    return paths;
}

/*
    Recursive mkdir, not written by us,
    https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
*/

void _mkdir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}

/*
    Recursive rmdir, not written by us,
    https://stackoverflow.com/a/2256974/14795595
*/

int _rmdir(const char *path) {
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d) {
        struct dirent *p;

        r = 0;
        while (!r && (p = readdir(d))) {
            int r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them.
             */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) continue;

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf) {
                struct stat statbuf;

                snprintf(buf, len, "%s/%s", path, p->d_name);
                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode))
                        r2 = _rmdir(buf);
                    else
                        r2 = unlink(buf);
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }

    if (!r) r = rmdir(path);

    return r;
}

char *get_username() {
    char *buf = malloc(256);
    strcpy(buf, getenv("USER"));
    return buf;
}
