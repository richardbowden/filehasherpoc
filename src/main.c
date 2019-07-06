#include <stdio.h>
#include <math.h>

#include <stdbool.h>
#include "file_scanner.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <unistd.h>

file_fifo_t *file_queue;

size_t file_counter = 0;
size_t dir_counter = 0;

// file_nod *files;

int cc = 0;

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
    if (!ff){
        fprintf(stderr, "root dir not set in -d option\n");
        exit(EXIT_FAILURE);
    }
    
    file_fifo_t file_queue;
    int res = fs_get_files(ff, &file_queue);

    if (!res)
    {
        fprintf(stderr, "fs_get_files failed, jerk\n");
        exit(EXIT_FAILURE);
    }

    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

    printf("actual hashing of %s took %f seconds to hash %d bytes\n", ff, time_taken, 0);

    //should free f but who cares, the OS will do then when the process ends, no point wasting cpu cycles todo somthing the OS can do better
    exit(EXIT_SUCCESS);
}
