#include "extractArchive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void extractArchive(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Invalid number of arguments for extracting.\n");
        exit(EXIT_FAILURE);
    }

    char *inputFileName = argv[2];
    char *outputDirectory = argv[3];

    FILE *archive = fopen(inputFileName, "rb");
    if (!archive)
    {
        perror("Error opening archive file");
        exit(EXIT_FAILURE);
    }

    size_t organizationSize;
    fscanf(archive, "%010zu", &organizationSize);

    if (mkdir(outputDirectory, 0755) == -1)
    {
        perror("Error creating output directory");
        exit(EXIT_FAILURE);
    }

    if (chdir(outputDirectory) == -1)
    {
        perror("Error changing directory");
        exit(EXIT_FAILURE);
    }

    fseek(archive, 10, SEEK_SET);
    char buffer[1024];
    fgets(buffer, sizeof(buffer), archive);

    char *token = strtok(buffer, "|");
    while (token != NULL)
    {
        FileInfo fileInfo;
        sscanf(token, "%[^,],%[^,],%zu", fileInfo.filename, fileInfo.permissions, &fileInfo.size);

        FILE *outputFile = fopen(fileInfo.filename, "wb");
        if (!outputFile)
        {
            perror("Error creating output file");
            exit(EXIT_FAILURE);
        }

        // Write the content
        for (size_t i = 0; i < fileInfo.size; ++i)
        {
            char c = fgetc(archive);
            fputc(c, outputFile);
        }

        fclose(outputFile);

        // Set permissions
        mode_t mode = 0;
        for (int i = 0; i < 9; i++)
        { // Permissions string is 9 characters
            if (fileInfo.permissions[i] != '-')
            {
                switch (i % 3)
                {
                case 0:
                    mode |= (1 << (8 - i));
                    break; // Read permissions
                case 1:
                    mode |= (1 << (8 - i));
                    break; // Write permissions
                case 2:
                    mode |= (1 << (8 - i));
                    break; // Execute permissions
                }
            }
        }
        chmod(fileInfo.filename, mode);

        token = strtok(NULL, "|");
    }
    fclose(archive);

    printf("Files opened in the %s directory.\n", outputDirectory);
}
