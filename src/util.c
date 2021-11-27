#include "util.h"

#include <stdio.h>
#include <string.h>

char *rev(char *s) {
    char *ptr1 = s;
    char *ptr2 = s + strlen(s) - 1;

    while (ptr2 > ptr1) {
        char t = *ptr1;
        *ptr1 = *ptr2;
        *ptr2 = t;
        ptr2--;
        ptr1++;
    }
    return s;
}