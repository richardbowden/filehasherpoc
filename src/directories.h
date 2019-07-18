#if !defined(DIRECTORIES_HEADER)
#define DIRECTORIES_HEADER
#include <stdbool.h>
#include "file_scanner.h"

int SyncDirMask_None;
int SyncDirMask_Recursive;

typedef struct sync_directory
{
    char *root;
    int options;

    size_t files_count;
    file_t *files[]; //array
} sync_directory;

sync_directory *sync_dir_new(char *root, size_t file_count, int options);
sync_directory *sync_dir_scan(char *root, int options);
// void sync_dir_add_contents(file_fifo_t *queue, sync_directory *sd);

#endif // DIRECTORIES_HEADER
