#include "textfile.h"
#include <stdio.h>
#include <stdlib.h>

int isTextFile(const char *filename)
{
    char command[256];
    snprintf(command, sizeof(command), "file %s | grep -qE 'text|empty'", filename);
    return system(command) == 0;
}

