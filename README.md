# SYSTEM PROGRAMMING 2023-2024 FALL TERM PROJECT

This program is a simple archive utility implemented in C such as .tar, .zip, etc. It provides basic functionality to create and extract archives containing text files. The archive format includes file metadata such as filename, permissions, and size, allowing for reconstruction of the original files.

# Usage
## Creating an Archive:
```bash
./tarsau -b file1 file2 ... -o output.sau
````
- '-b': Indicates the archive creation mode.
- file1 file2 ...: List of text files to be archived.
- '-o output.sau': Specifies the output archive file.

## Extracting from an Archive:
```bash
./tarsau -a output.sau output_directory
````
- 'a': Indicates the archive extraction mode.
- 'input.sau: The input archive file.
- output_directory: The directory where the extracted files will be stored.

# Limitations
- The program assumes that the input files are text files. It uses the file command to check if a file is text or binary.
- The maximum size of input files combined should not exceed 200 MB (configurable with MAX_SIZE).
- The permissions of the extracted files might not be preserved perfectly, as it depends on the capabilities of the file system.

# Implementation Details
## isTextFile Function
Uses the file command to determine if a file is a text file. The result is based on whether the output contains the words 'text' or 'empty.'

## Archive Creation (createArchive Function)
- Parses command line arguments to extract input filenames and the output archive filename.
- Checks if each input file is a text file using the isTextFile function.
- Retrieves file information such as filename, permissions, and size.
- Calculates the total size of input files and checks if it exceeds the limit.
- Creates an archive file and writes metadata (filename, permissions, size) and file content sequentially.

## Archive Extraction (extractArchive Function)
- Parses command line arguments to extract the input archive filename and the output directory.
- Opens the archive file and retrieves the total size of the organization and metadata.
- Creates the output directory and changes the current working directory.
- Reads metadata from the archive and reconstructs files with their content and permissions.

# Requirements
- For simplicity, the program assumes a Linux-like environment with POSIX-compliant file systems.
- The program uses file and grep commands to determine if a file is a text file. Ensure these commands are available on your system.






