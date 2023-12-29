#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILES 32
#define MAX_SIZE 200 * 1024 * 1024 // 200 MB

typedef struct {
    char filename[256];
    char permissions[10];
    size_t size;
} FileInfo;

void createArchive(int argc, char *argv[]);
void extractArchive(int argc, char *argv[]);
int isTextFile(const char *filename);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s -b file1 file2 ... -o output.sau\n", argv[0]);
        fprintf(stderr, "       %s -a input.sau output_directory\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-b") == 0) {
        createArchive(argc, argv);
    } else if (strcmp(argv[1], "-a") == 0) {
        extractArchive(argc, argv);
    } else {
        fprintf(stderr, "Invalid option: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}

int isTextFile(const char *filename) {
    char command[256];
    snprintf(command, sizeof(command), "file %s | grep -qE 'text|empty'", filename);

    return system(command) == 0;
}


void createArchive(int argc, char *argv[]) {
    char *outputFileName = "a.sau";
    int fileCount = 0;
    FileInfo fileInfos[MAX_FILES];

    // Parse command line arguments
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outputFileName = argv[i + 1];
            i++;  // Skip the next argument, which is the output file name
        } else {
            // Check if the file is a text file
            if (!isTextFile(argv[i])) {
                fprintf(stderr, "File %s is not a text file.\n", argv[i]);
                exit(EXIT_FAILURE);
            }

            // Store file information
            strncpy(fileInfos[fileCount].filename, argv[i], sizeof(fileInfos[fileCount].filename) - 1);
            fileInfos[fileCount].filename[sizeof(fileInfos[fileCount].filename) - 1] = '\0';

            struct stat fileStat;
            if (stat(argv[i], &fileStat) == -1) {
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
                (fileStat.st_mode & S_IXOTH) ? 'x' : '-'
            );

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
    for (int i = 0; i < fileCount; ++i) {
        totalSize += fileInfos[i].size;
    }

    // Check if the total size exceeds the limit
    if (totalSize > MAX_SIZE) {
        fprintf(stderr, "Total size of input files exceeds the limit.\n");
        exit(EXIT_FAILURE);
    }

    // Create or overwrite the archive file
    FILE *archive = fopen(outputFileName, "wb");
    if (!archive) {
        perror("Error creating archive file");
        exit(EXIT_FAILURE);
    }

    
    fprintf(archive, "%010zu|", ftell(archive) + fileCount * (sizeof(FileInfo) + 3));
    for (int i = 0; i < fileCount; ++i) {
        fprintf(archive, "%s,%s,%zu|", fileInfos[i].filename, fileInfos[i].permissions, fileInfos[i].size);
    }


    fprintf(archive, "\n");

    // Write the archived files to the archive file
    for (int i = 0; i < fileCount; ++i) {
        FILE *currentFile = fopen(fileInfos[i].filename, "rb");
        if (!currentFile) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

     
        size_t bytesRead;
        unsigned char buffer[1024];
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), currentFile)) > 0) {
            fwrite(buffer, 1, bytesRead, archive);
        }

        fclose(currentFile);
    }

    fclose(archive);

    printf("The files have been merged.\n");
}

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
    fscanf(archive, "%010zu", &organizationSize);

    if (mkdir(outputDirectory, 0755) == -1) {
        perror("Error creating output directory");
        exit(EXIT_FAILURE);
    }

    if (chdir(outputDirectory) == -1) {
        perror("Error changing directory");
        exit(EXIT_FAILURE);
    }

    fseek(archive, 10, SEEK_SET);
    char buffer[1024];
    fgets(buffer, sizeof(buffer), archive);

    char *token = strtok(buffer, "|");
    while (token != NULL) {
        FileInfo fileInfo;
        sscanf(token, "%[^,],%[^,],%zu", fileInfo.filename, fileInfo.permissions, &fileInfo.size);

        FILE *outputFile = fopen(fileInfo.filename, "wb");
        if (!outputFile) {
            perror("Error creating output file");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < fileInfo.size; ++i) {
            char c = fgetc(archive);
            fputc(c, outputFile);
        }

        fclose(outputFile);

        token = strtok(NULL, "|");
    }

    fclose(archive);

    printf("Files opened in the %s directory.\n", outputDirectory);
}

