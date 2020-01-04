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

#include "librb.h"

int errno;

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

    char *jjj = expand_home_dir("~/bin");
    printf("%s\n", jjj);
    
    get_hostname();
    db_init();
    
    clock_t t;
    t = clock();

    char *ff = NULL;
    char *set_name = NULL;
    int opt;
    
    bool compare = false;
    char *primary;
    char *secondary;
    
    while ((opt = getopt(argc, argv, "d:r:s:cP:S:")) != -1)
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
            sync_dir_read_file(ff, &sd);
            db_sd_import(sd);
            exit(EXIT_SUCCESS);
            break;
        case 'c':
            printf("file compare - primary against secondary\n");
            compare = true;
            break;
        case 's':
            set_name = strdup(optarg);
            break;
                
        case 'P':
            primary = strdup(optarg);
            break;
                
        case 'S':
            secondary = strdup(optarg);
            break;
        default:
            printf("invalid arg\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

//    if (compare){
//        db_init();
//        int res;
//        res = hblk_import(primary);
//        res = hblk_import(secondary);
//        res = hblk_compare();
//    }
    
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
    
    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

    DEBUG_PRINT("full scan took: %f", time_taken);

    sync_dir_write_file("testdump.hblk", sd);

    exit(EXIT_SUCCESS);
}

