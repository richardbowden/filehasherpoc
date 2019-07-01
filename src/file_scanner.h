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
    const char *file;              /**full paht to the file */
    struct stat f_info;            /**file info */
    size_t block_size;             /**the block_size selected for this file */
    size_t size;                   /**total file size */
    size_t aligned_size;           /**size that is divisiable by block_size */
    size_t aligned_chunks;         /**number of chunks */
    size_t last_chunk_size;        /**the remaining size if not aligned */
    size_t last_chunk_offset;      /**where the last chunk starts */
    size_t last_chunk_offset_size; /**the size size of the last chunk calculated from the offset */
    bool aligned;                  /**set if size is not divisable by block_size */
    block_t *blocks;               /**array of hashes of each block */

    struct file_s *next;
    struct file_s *prev;
} file_t;

typedef struct file_fifo_s
{
    struct file_s *head;
    struct file_s *tail;
} file_fifo_t;

/** creates a file_t object with file which is a full path to a file*/
file_t *new_file(char *file);

/** takes a file_t, and calculates the file size and cchunking info */
file_t *populate_file_stats(const char *file);

/** hash_file takes a file_t and actually performs the hash calc */
void hash_file(file_t *file);

/** linked list functions */

/**new_file_fifo creates a new foe_lost */
file_fifo_t *new_file_fifo(void);
// void push_file(file_node **head_ref, file_t *file);

// FIFO
void add_to_file_fifo(file_fifo_t *list, file_t *file);
file_t *pop_file_from_list(file_fifo_t *list);

#endif
