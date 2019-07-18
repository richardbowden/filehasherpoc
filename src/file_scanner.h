#ifndef FILE_SCANNER
#define FILE_SCANNER

#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>

size_t BLOCK_SIZE;

/*! \brief block_s.
        Holds the block hashes for a file
 *
 *  128bit hash for each chunk_size
 */
typedef struct block_s
{
    size_t offset; //where the block starts
    int hash[4];   //the hash, its 128bit split in to 4, see https://github.com/cmuratori/meow_hash
} block_t;

/**
 * file_s holds data about file and its size and chunks
 */
typedef struct file_s
{
    const char *file;   /**full paht to the file */
    struct stat f_info; /**file info */

    size_t block_size;   /**the block_size selected for this file */
    size_t size;         /**total file size */
    size_t aligned_size; /**size that is divisiable by block_size */

    size_t last_block_size;   /**the remaining size if not aligned */
    size_t last_block_offset; /**where the last chunk starts */

    bool aligned; /**set if size is not divisable by block_size */
    bool below_block;
    block_t *blocks; /**array of hashes of each block */
    size_t block_count;

    double hash_scan_time; /**number of seconds it took to hash the file */

    struct file_s *next;
    struct file_s *prev;
} file_t;

typedef struct file_fifo_s
{
    struct file_s *head;
    struct file_s *tail;
    size_t count;
} file_fifo_t;

/** creates a file_t object with file which is a full path to a file*/
file_t *new_file(const char *file);

/** takes a file_t, and calculates the file size and cchunking info */
void populate_file_stats(file_t *file);

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
