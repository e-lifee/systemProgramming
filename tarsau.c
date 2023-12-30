#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_FILES 32
#define MAX_SIZE 200 * 1024 * 1024 // 200 MB

typedef struct
{
    char filename[256];
    char permissions[10];
    size_t size;
} FileInfo;

void createArchive(int argc, char *argv[]);
void extractArchive(int argc, char *argv[]);
int isTextFile(const char *filename);

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

int isTextFile(const char *filename)
{
    char command[256];
    snprintf(command, sizeof(command), "file %s | grep -qE 'text|empty'", filename);

    return system(command) == 0;
}

void createArchive(int argc, char *argv[])
{
    char *outputFileName = "a.sau";
    int fileCount = 0;
    FileInfo fileInfos[MAX_FILES];

    for (int i = 2; i < argc; ++i)
    {
        char fullPath[260];
        snprintf(fullPath, sizeof(fullPath), "txt/%s", argv[i]);

        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            outputFileName = argv[i + 1];
            i++;
        }
        else
        {
            if (!isTextFile(fullPath))
            {
                fprintf(stderr, "File %s is not a text file.\n", fullPath);
                exit(EXIT_FAILURE);
            }

            strncpy(fileInfos[fileCount].filename, fullPath, sizeof(fileInfos[fileCount].filename) - 1);
            fileInfos[fileCount].filename[sizeof(fileInfos[fileCount].filename) - 1] = '\0';

            struct stat fileStat;
            if (stat(fullPath, &fileStat) == -1)
            {
                perror("Error getting file information");
                exit(EXIT_FAILURE);
            }

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

            fileInfos[fileCount].size = fileStat.st_size;

            fileCount++;
        }
    }

    size_t totalSize = 0;
    for (int i = 0; i < fileCount; ++i)
    {
        totalSize += fileInfos[i].size;
    }

    if (totalSize > MAX_SIZE)
    {
        fprintf(stderr, "Total size of input files exceeds the limit.\n");
        exit(EXIT_FAILURE);
    }

    FILE *archive = fopen(outputFileName, "wb");
    if (!archive)
    {
        perror("Error creating archive file");
        exit(EXIT_FAILURE);
    }

    fprintf(archive, "%010zu|", totalSize);
    for (int i = 0; i < fileCount; ++i)
    {
        fprintf(archive, "%s,%s,%zu|", fileInfos[i].filename, fileInfos[i].permissions, fileInfos[i].size);
    }

    fprintf(archive, "\n");

    for (int i = 0; i < fileCount; ++i)
    {
        FILE *currentFile = fopen(fileInfos[i].filename, "rb");
        if (!currentFile)
        {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        unsigned char buffer[1024];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), currentFile)) > 0)
        {
            fwrite(buffer, 1, bytesRead, archive);
        }

        fclose(currentFile);
    }

    fclose(archive);
    printf("The files have been merged.\n");
}

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
    fscanf(archive, "%010zu|", &organizationSize);

    if (mkdir(outputDirectory, 0755) == -1 && errno != EEXIST)
    {
        perror("Error creating output directory");
        exit(EXIT_FAILURE);
    }

    if (chdir(outputDirectory) == -1)
    {
        perror("Error changing directory");
        exit(EXIT_FAILURE);
    }

    fseek(archive, 11, SEEK_SET);
    char line[1024];
    while (fgets(line, sizeof(line), archive) != NULL)
    {
        char *token = strtok(line, "|");
        while (token != NULL && strcmp(token, "\n") != 0)
        {
            FileInfo fileInfo;
            if (sscanf(token, "%[^,],%[^,],%zu", fileInfo.filename, fileInfo.permissions, &fileInfo.size) != 3)
            {
                fprintf(stderr, "Error parsing file info: %s\n", token);
                exit(EXIT_FAILURE);
            }

            char fullPath[260];
            snprintf(fullPath, sizeof(fullPath), "txt/%s", fileInfo.filename);

            FILE *outputFile = fopen(fullPath, "wb");
            if (!outputFile)
            {
                perror("Error creating output file");
                exit(EXIT_FAILURE);
            }

            for (size_t i = 0; i < fileInfo.size; i++)
            {
                int c = fgetc(archive);
                if (c == EOF)
                {
                    fprintf(stderr, "Unexpected end of file\n");
                    exit(EXIT_FAILURE);
                }
                fputc(c, outputFile);
            }

            fclose(outputFile);

            mode_t mode = 0;
            for (int i = 0; i < 9; i++)
            {
                if (fileInfo.permissions[i] != '-')
                {
                    mode |= (1 << (8 - i));
                }
            }
            chmod(fullPath, mode);

            token = strtok(NULL, "|");
        }
    }
    fclose(archive);

    printf("Files opened in the %s directory.\n", outputDirectory);
}
