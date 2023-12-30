#include "extractArchive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

void extractArchive(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Invalid number of arguments for extracting.\n");
        exit(EXIT_FAILURE);
    }

    char *inputFileName = argv[2];
    char *outputDirectory = argv[3];

    FILE *archive = fopen(inputFileName, "rb");
    if (!archive) {
        perror("Error opening archive file");
        exit(EXIT_FAILURE);
    }

    size_t organizationSize;
    fscanf(archive, "%010zu|", &organizationSize);

    if (mkdir(outputDirectory, 0755) == -1 && errno != EEXIST) {
        perror("Error creating output directory");
        exit(EXIT_FAILURE);
    }

    if (chdir(outputDirectory) == -1) {
        perror("Error changing directory");
        exit(EXIT_FAILURE);
    }

    fseek(archive, 11, SEEK_SET); // Skip past the organization size
    char line[1024];
    while (fgets(line, sizeof(line), archive) != NULL) {
        char *token = strtok(line, "|");
        while (token != NULL && strcmp(token, "\n") != 0) {
            FileInfo fileInfo;
            if (sscanf(token, "%[^,],%[^,],%zu", fileInfo.filename, fileInfo.permissions, &fileInfo.size) != 3) {
                fprintf(stderr, "Error parsing file info: %s\n", token);
                exit(EXIT_FAILURE);
            }

            // Validate filename
            if (strchr(fileInfo.filename, '/') || strstr(fileInfo.filename, "..")) {
                fprintf(stderr, "Invalid filename in archive: %s\n", fileInfo.filename);
                exit(EXIT_FAILURE);
            }

            FILE *outputFile = fopen(fileInfo.filename, "wb");
            if (!outputFile) {
                perror("Error creating output file");
                exit(EXIT_FAILURE);
            }

            for (size_t i = 0; i < fileInfo.size; i++) {
                int c = fgetc(archive);
                if (c == EOF) {
                    fprintf(stderr, "Unexpected end of file\n");
                    exit(EXIT_FAILURE);
                }
                fputc(c, outputFile);
            }

            fclose(outputFile);

            // Set permissions
            mode_t mode = 0;
            for (int i = 0; i < 9; i++) {
                if (fileInfo.permissions[i] != '-') {
                    mode |= (1 << (8 - i));
                }
            }
            chmod(fileInfo.filename, mode);

            token = strtok(NULL, "|");
        }
        if (token && strcmp(token, "\n") != 0) {
            // Handle case where last file info is followed by EOF without newline
            break;
        }
    }    fclose(archive);

    printf("Files opened in the %s directory.\n", outputDirectory);
}
