#ifndef FILE_SCANNER
#define FILE_SCANNER

#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "meowhash.h"

enum block_mode_flags{
     BM_BLOCKS       = 0x1,
     BM_FILE         = 0x2,
    
     BM_HASH_MEOW    = 0x200,
};

size_t BLOCK_SIZE;

/*! \brief block_s.
        Holds the block hashes for a file
 *
 *  128bit hash for each chunk_size
 */
typedef struct block_s
{
    size_t offset;
    int mode;
    long raw_high;
    long raw_low;
    char str[33];
} block_t;

int FileTypeFile;
int FileTypeDir;

/**
 * file_s holds data about file and its size and chunks
 */
typedef struct file_s
{
    char *file_abs; //** absolute path to the file */
    char *file_rel; /** relative path to the file, minus base path */

    // struct stat f_info;  /**file info */
    int type;          /** file or directory or maybe symlink */
    size_t block_size; /**the block_size selected for this file */
    // size_t size;       /**total file size */

    //############################################################
    //################  file info - from stat  ###################
    //############################################################
    size_t size; /** total file size */
    uid_t uid;   /** User ID */
    gid_t gid;   /** Group ID */

//TODO:(rich) abstract this to our tie values, so we do not need custom code for each platforms time def
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
    struct timespec atimespec; /* time of last access */
    struct timespec mtimespec; /* last data modification */
    struct timespec ctimespec; /* last status change*/
#else
#error add a, m and c time for linux
#endif
    //###########################################################

    double hash_scan_time; /**number of seconds it took to hash the file */

    struct file_s *next;
    struct file_s *prev;

    block_t whole_file_hash;
    
    size_t block_count;
    block_t *blocks; /**array of hashes of each block */

} file_t;

typedef struct file_fifo_s
{
    struct file_s *head;
    struct file_s *tail;
    size_t count;
} file_fifo_t;

/** creates a file_t object with file which is a full path to a file*/
file_t *new_file(const char *file, const struct stat *f_stat);

/** takes a file_t, and calculates the file size and cchunking info */
// void populate_file_stats(file_t *file);

/** hash_file takes a file_t and actually performs the hash calc */
void hash_file(file_t *file);

/** linked list functions */

/**new_file_fifo creates a new foe_lost */
file_fifo_t *new_file_fifo(void);
// void push_file(file_node **head_ref, file_t *file);

// FIFO
void file_fifo_add(file_fifo_t *list, file_t *file);
file_t *fs_fifo_pop(file_fifo_t *list);

int (*file_handle)(const char *f, const struct stat *f_stat, int i); // = file_handler;

file_fifo_t *scan_files(char *root);

//public interfaces
int fs_get_files(char *root_dir, file_fifo_t *queue);

#endif
