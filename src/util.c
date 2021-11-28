#include "util.h"

#include <stdio.h>
#include <string.h>

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