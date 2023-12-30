#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <stddef.h> 

#define MAX_FILENAME_LENGTH 256
#define MAX_PERMISSIONS_LENGTH 10
#define MAX_FILES 32
#define MAX_SIZE (200 * 1024 * 1024) // 200 MB

typedef struct
{
    char filename[MAX_FILENAME_LENGTH];
    char permissions[MAX_PERMISSIONS_LENGTH];
    size_t size;
} FileInfo;

#endif 

