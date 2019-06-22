#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "file_scanner.h"
#include "meowhash.h"
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

    FILE *fp;
    fp = fopen(f->file, "rb");

    if (fp == NULL)
    {
        fprintf(stderr, "failed to open file: %s\n", file);
        exit(EXIT_FAILURE);
    }

    size_t cur_bytes_read = 0;
    size_t total_read = 0;
    size_t block_counter = 0;
    char buffer[BLOCK_SIZE]; //on the stack so its fast

    while ((cur_bytes_read = fread(buffer, sizeof(char), BLOCK_SIZE, fp)) > 0)
    {

        meow_u128 Hash = MeowHash(MeowDefaultSeed, BLOCK_SIZE, buffer);
        f->blocks[block_counter].offset = total_read;
        f->blocks[block_counter].hash[3] = MeowU32From(Hash, 3);
        f->blocks[block_counter].hash[2] = MeowU32From(Hash, 2);
        f->blocks[block_counter].hash[1] = MeowU32From(Hash, 1);
        f->blocks[block_counter].hash[0] = MeowU32From(Hash, 0);

        total_read += cur_bytes_read;
        block_counter += 1;
    }

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
    
    char *si;
    readable_fs(f->size, si);
    printf("actual hashing of %s took %f seconds to hash %s \n", f->file, time_taken, si);
    fclose(fp);
    //should free f but who cares, the OS will do then when the process ends, no point wasting cpu cycles todo somthing the OS can do better
    return 0;
}
