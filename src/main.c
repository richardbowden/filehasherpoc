#include <stdio.h>
#include <math.h>

#include <stdbool.h>
#include "file_scanner.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "directories.h"
#include <unistd.h>
#include "debug.h"
#include "directory_hblk.h"
#include <unistd.h>
#include "database.h"
#include <time.h>

char *hostname;

size_t file_counter = 0;
size_t dir_counter = 0;

int cc = 0;

void get_hostname(){
    hostname = (char*)calloc(1, _SC_HOST_NAME_MAX+1);
    gethostname(hostname, _SC_HOST_NAME_MAX+1);
}

int main(int argc, char *argv[])
{
    
    get_hostname();
    db_init();
    
    clock_t t;
    t = clock();

    char *ff = NULL;
    char *set_name = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "d:r:s:")) != -1)
    {
        switch (opt)
        {

        case 'd':
            printf("f:%s\n", optarg);
            ff = strdup(optarg);
            break;
        case 'r':
            ff = strdup(optarg);
            sync_directory *sd = NULL;
            sync_dir_read_file(ff, sd);
                exit(EXIT_SUCCESS);
            break;
        case 's':
            set_name = strdup(optarg);
            break;
        default:
            printf("invalid arg\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (!ff)
    {
        fprintf(stderr, "root dir not set in -d option\n");
        exit(EXIT_FAILURE);
    }
    
    if (!set_name){
        set_name = strdup("default");
    }
    
    printf("set_name: %s\n", set_name);

    sync_directory *sd;
    time_t started = time(NULL);
    sd = sync_dir_scan(ff, SyncDirMask_Recursive);
    if (sd == NULL)
    {
        fprintf(stderr, "sync_dir_scan failed, jerk\n");
        exit(EXIT_FAILURE);
    }
    
    sd->hostname = strdup(hostname);
    sd->set_name = strdup(set_name);
    sd->started_at = started;
    sd->finished_at = time(NULL);
    
//    db_add_set(sd);
    
    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

    DEBUG_PRINT("full scan took: %f", time_taken);

    sync_dir_write_file("testdump.hblk", sd);
	

    // sync_directory *sd = new_sync_dir(ff, file_queue.count, SyncDirMask_Recursive);
    // sync_dir_add_contents(&file_queue, sd);

    //    file_t *cur_file;
    //    cur_file = file_queue.head;
    //
    //    char *h = (char *)malloc(sizeof(char) * 36 + 1);
    //
    //    while (cur_file != NULL)
    //    {
    //
    //        for (int cc = 0; cc < cur_file->block_count; cc++)
    //        {
    //            //            char h[];
    //            sprintf(h, "%08X-%08X-%08X-%08X",
    //                    cur_file->blocks[cc].hash[3],
    //                    cur_file->blocks[cc].hash[2],
    //                    cur_file->blocks[cc].hash[1],
    //                    cur_file->blocks[cc].hash[0]);
    //
    //            printf("%s, block: %d, hash: %s, size: %lld, block_size: %zu\n", cur_file->file, cc, h, cur_file->f_info.st_size, cur_file->block_size);
    //        }
    //
    //        cur_file = cur_file->next;
    //    }
    //
    //    printf("actual hashing of %s took %f seconds to hash %d bytes\n", ff, time_taken, 0);

    //should free f but who cares, the OS will do then when the process ends, no point wasting cpu cycles todo somthing the OS can do better
    exit(EXIT_SUCCESS);
}

//char *get_hash(block_t *block)
//{
//
//    printf("    %08X-%08X-%08X-%08X\n",
//           MeowU32From(block, 3),
//           MeowU32From(Hash, 2),
//           MeowU32From(Hash, 1),
//           MeowU32From(Hash, 0));
//}
