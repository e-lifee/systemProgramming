#ifndef EXTRACT_ARCHIVE_H
#define EXTRACT_ARCHIVE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fileInfo.h"

void extractArchive(int argc, char *argv[]);

#endif 
