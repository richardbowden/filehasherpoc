#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "file_scanner.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    char *file;
    if (argc == 2)
    {
        file = strdup(argv[1]);
    }
    else
    {
        fprintf(stderr, "first arg must be a file to hash\n");
        exit(1);
    };

    clock_t t;
    t = clock();

    file_t *f = populate_file_stats(file);
    if (!f)
    {
        fprintf(stderr, "you did not enter a valid file and/or path: %s\n", file);
        exit(1);
    }

    hash_file(f);

    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

    size_t chunks = f->aligned_chunks;
    if (!f->aligned)
    {
        chunks++;
    }

    printf("Hash Dump Start");

    for (size_t i = 0; i < chunks; i++)
    {
        printf("block: %ld, offset: %ld, hash: %08X-%08X-%08X-%08X\n",
               i,
               f->blocks[i].offset,
               f->blocks[i].hash[3],
               f->blocks[i].hash[2],
               f->blocks[i].hash[1],
               f->blocks[i].hash[0]);
    }

    printf("Hash Dump Finish\n\n");

    // char *si;
    // readable_fs(f->size, &si);
    printf("actual hashing of %s took %f seconds to hash %zu bytes\n", f->file, time_taken, f->size);

    //should free f but who cares, the OS will do then when the process ends, no point wasting cpu cycles todo somthing the OS can do better
    return 0;
}
