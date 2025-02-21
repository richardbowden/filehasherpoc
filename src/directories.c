#include <stdlib.h>

#include "directories.h"
#include "debug.h"
#include <time.h>
#include <string.h>
#include <assert.h>

int SyncDirMask_None = 1 << 0;
int SyncDirMask_Recursive = 1 << 1;


sync_directory *sync_dir_new(char *root, size_t file_count, int options)
{
    size_t s = sizeof(sync_directory) + (file_count * sizeof(file_t));

    sync_directory *sd = calloc(1, s);
    sd->files_count = file_count;
    sd->option_flags = options;
    sd->root = strdup(root);
    return sd;
}

sync_directory *sync_dir_scan(char *root, int options)
{
    //start the directory scan
    file_fifo_t queue = {};
    fs_get_files(root, &queue);

    if (queue.count == 0)
    {
        return NULL;
    }

    sync_directory *sd = sync_dir_new(root, queue.count, SyncDirMask_Recursive);

    file_t *cur_file;
    size_t counter = 0;

    clock_t t, s;
    s = clock();

    cur_file = fs_fifo_pop(&queue);

    assert(cur_file->file_abs[0] == '/');

    while (cur_file != NULL)
    {
        DEBUG_PRINT("%s\n", cur_file->file_abs);

        sd->files[counter] = cur_file;
        counter++;
        cur_file = fs_fifo_pop(&queue);
    }

    t = clock() - s;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

    // DEBUG_PRINT("time_taken: %f", time_taken);

    return sd;
}

// void sync_dir_add_contents(file_fifo_t *queue, sync_directory *sd)
// {
//     file_t *cur_file;
//     cur_file = queue->head;
//     size_t counter = 0;

//     while (cur_file != NULL)
//     {
//         sd->files[counter] = *cur_file;
//         counter++;
//         cur_file = cur_file->next;
//     }
// }
