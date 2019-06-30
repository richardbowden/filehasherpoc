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

} file_t;

/**file node for linked list */
typedef struct file_node
{
    file_t *file;
    struct file_node *next;
    struct file_node *prev;
} file_node;

typedef struct file_list
{
    file_node *head;
    file_node *tail;
} file_list;

/**new_file_list init's a new linked list */
file_list *new_file_list(void);

file_t *new_file(char *file);
file_t *populate_file_stats(const char *file);

void hash_file(file_t *file);
void push_file(file_node **head_ref, file_t *file);

void push_file_to_list(file_list *list, file_t *file);
#endif
