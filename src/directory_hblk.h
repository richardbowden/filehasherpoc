#if !defined(DIRECTORIES_HBLK_HEADER)
#define DIRECTORIES_HBLK_HEADER

#include <stdlib.h>
#include "directories.h"

int SyncDirMajor;
int SyncDirMinor;

size_t sync_dir_write_file(char *file, sync_directory *sd);

#endif
