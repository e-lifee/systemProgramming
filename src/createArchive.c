#include "createArchive.h"
#include "textfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


void createArchive(int argc, char *argv[])
{
    char *outputFileName = "a.sau";
    int fileCount = 0;
    FileInfo fileInfos[MAX_FILES];

    // Parse command line arguments
    for (int i = 2; i < argc; ++i)
    {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            outputFileName = argv[i + 1];
            i++; // Skip the next argument, which is the output file name
        }
        else
        {
            // Check if the file is a text file
            if (!isTextFile(argv[i]))
            {
                fprintf(stderr, "File %s is not a text file.\n", argv[i]);
                exit(EXIT_FAILURE);
            }

            // Store file information
            strncpy(fileInfos[fileCount].filename, argv[i], sizeof(fileInfos[fileCount].filename) - 1);
            fileInfos[fileCount].filename[sizeof(fileInfos[fileCount].filename) - 1] = '\0';

            struct stat fileStat;
            if (stat(argv[i], &fileStat) == -1)
            {
                perror("Error getting file information");
                exit(EXIT_FAILURE);
            }

            // Convert mode to permission string
            snprintf(fileInfos[fileCount].permissions, sizeof(fileInfos[fileCount].permissions), "%c%c%c%c%c%c%c%c%c",
                     (fileStat.st_mode & S_IRUSR) ? 'r' : '-',
                     (fileStat.st_mode & S_IWUSR) ? 'w' : '-',
                     (fileStat.st_mode & S_IXUSR) ? 'x' : '-',
                     (fileStat.st_mode & S_IRGRP) ? 'r' : '-',
                     (fileStat.st_mode & S_IWGRP) ? 'w' : '-',
                     (fileStat.st_mode & S_IXGRP) ? 'x' : '-',
                     (fileStat.st_mode & S_IROTH) ? 'r' : '-',
                     (fileStat.st_mode & S_IWOTH) ? 'w' : '-',
                     (fileStat.st_mode & S_IXOTH) ? 'x' : '-');

            fileInfos[fileCount].permissions[sizeof(fileInfos[fileCount].permissions) - 1] = '\0';

            FILE *currentFile = fopen(argv[i], "rb");
            fseek(currentFile, 0, SEEK_END);
            fileInfos[fileCount].size = ftell(currentFile);
            fclose(currentFile);

            fileCount++;
        }
    }

    // Calculate total size of input files
    size_t totalSize = 0;
    for (int i = 0; i < fileCount; ++i)
    {
        totalSize += fileInfos[i].size;
    }

    // Check if the total size exceeds the limit
    if (totalSize > MAX_SIZE)
    {
        fprintf(stderr, "Total size of input files exceeds the limit.\n");
        exit(EXIT_FAILURE);
    }

    // Create or overwrite the archive file
    FILE *archive = fopen(outputFileName, "wb");
    if (!archive)
    {
        perror("Error creating archive file");
        exit(EXIT_FAILURE);
    }

    fprintf(archive, "%010zu|", ftell(archive) + totalSize);
    for (int i = 0; i < fileCount; ++i)
    {
        fprintf(archive, "%s,%s,%zu|", fileInfos[i].filename, fileInfos[i].permissions, fileInfos[i].size);
    }

    fprintf(archive, "\n");

    // Write the archived files to the archive file
    for (int i = 0; i < fileCount; ++i)
    {
        FILE *currentFile = fopen(fileInfos[i].filename, "rb");
        if (!currentFile)
        {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        size_t bytesRead;
        unsigned char buffer[1024];
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), currentFile)) > 0)
        {
            fwrite(buffer, 1, bytesRead, archive);
        }

        fclose(currentFile);
    }

    fclose(archive);

    printf("The files have been merged.\n");
}

