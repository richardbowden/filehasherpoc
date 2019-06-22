#include "file_scanner.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#define KILOBYTE 1024
#define MEGABYTE (KILOBYTE * KILOBYTE)
#define GIGABYTE (MEGABYTE * KILOBYTE)
char buffer[64 * KILOBYTE];

// Data to create dynamic block sizes depending on the size of the file
//Block Sizes
//---------------------------------
// File Size	        Block Size
// 0 - 250 MiB	        128 KiB
// 250 MiB - 500 MiB	256 KiB
// 500 MiB - 1 GiB	    512 KiB
// 1 GiB - 2 GiB	    1 MiB
// 2 GiB - 4 GiB	    2 MiB
// 4 GiB - 8 GiB	    4 MiB
// 8 GiB - 16 GiB	    8 MiB
// 16 GiB - up	        16 MiB

size_t BLOCK_SIZE = 64 * KILOBYTE;

file_t *new_file(char *file)
{
    file_t *f;
    f = calloc(1, sizeof(file_t));
    f->file = strdup(file);
    return f;
}

file_t *populate_file_stats(char *file)
{
    struct stat f_info;

    int err = stat(file, &f_info);

    if (err != 0)
    {
        fprintf(stderr, "failed to stat file\n");
        return NULL;
    }

    int aligned_chunks = f_info.st_size / BLOCK_SIZE;
    int last_chunk_size = f_info.st_size % BLOCK_SIZE;

    size_t num_of_blocks = aligned_chunks;

    if (last_chunk_size != 0)
    {
        num_of_blocks += 1;
    }

    file_t *encapped_file;
    size_t ss = sizeof(file_t) + sizeof(block_t);
    encapped_file = calloc(1, ss);

    printf("size of encapped_file: %ld\n", sizeof(encapped_file));

    encapped_file->file = strdup(file);
    encapped_file->size = f_info.st_size;

    encapped_file->aligned_chunks = aligned_chunks;
    encapped_file->aligned_size = aligned_chunks * BLOCK_SIZE;
    encapped_file->aligned = true;

    if (last_chunk_size != 0)
    {
        encapped_file->last_chunk_size = last_chunk_size;
        encapped_file->last_chunk_offset = encapped_file->aligned_size + 1;
        encapped_file->last_chunk_offset_size = last_chunk_size - 1;
        encapped_file->aligned = false;
    }

    block_t *b = malloc(num_of_blocks * sizeof(block_t));
    encapped_file->blocks = b;
    return encapped_file;
}

void readable_fs(double size /*in bytes*/, char *buf)
{
    int i = 0;
    const char *units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1024)
    {
        size /= 1024;
        i++;
    }
    sprintf(buf, "%.*f %s", i, size, units[i]);
}