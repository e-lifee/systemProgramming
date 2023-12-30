#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileInfo.h"
#include "createArchive.h"
#include "extractArchive.h"
#include "textfile.h"
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s -b file1 file2 ... -o output.sau\n", argv[0]);
        fprintf(stderr, "       %s -a input.sau output_directory\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-b") == 0)
    {
        createArchive(argc, argv);
    }
    else if (strcmp(argv[1], "-a") == 0)
    {
        extractArchive(argc, argv);
    }
    else
    {
        fprintf(stderr, "Invalid option: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}

