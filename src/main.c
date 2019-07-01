#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "file_scanner.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ftw.h>

#include <unistd.h>

file_fifo_t *file_queue;

size_t file_counter = 0;
size_t dir_counter = 0;

// file_nod *files;

int cc = 0;

int file_handler(const char *f, const struct stat *f_stat, int i)
{
    switch (i)
    {
    case FTW_D:
        dir_counter++;
        break;
    case FTW_F:
        file_counter++;
        cc++;
        file_t *file;
        file = populate_file_stats(f);
        // push_file(&files, file);

        add_to_file_fifo(file_queue, file);

    default:
        break;
    }

    return 0;
}

int (*file_handle)(const char *f, const struct stat *f_stat, int i) = file_handler;

int main(int argc, char *argv[])
{

    file_queue = calloc(1, sizeof(file_queue));

    clock_t t;
    t = clock();
    // files = calloc(1, sizeof(file_node));

    char *ff;
    int opt;
    while ((opt = getopt(argc, argv, "d:")) != -1)
    {
        switch (opt)
        {

        case 'd':
            printf("f:%s\n", optarg);
            ff = strdup(optarg);
            break;

        default:
            printf("invalid arg\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

    ftw(ff, file_handle, 16);

    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

    printf("actual hashing of %s took %f seconds to hash %d bytes\n", ff, time_taken, 0);

    //should free f but who cares, the OS will do then when the process ends, no point wasting cpu cycles todo somthing the OS can do better
    return 0;
}
