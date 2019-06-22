#ifndef FILE_SCANNER
#define FILE_SCANNER
#include <stdio.h>
#include <stdbool.h>

size_t BLOCK_SIZE;

typedef struct block_s
{
    size_t offset; //where the block starts
    int hash[4];   //the hash, its 128bit split in to 4, see https://github.com/cmuratori/meow_hash
} block_t;

typedef struct file_s
{
    const char *file;              //full paht to the file
    size_t block_size;             //the block_size selected for this file
    size_t size;                   //total file size
    size_t aligned_size;           //size that is divisiable by block_size
    size_t aligned_chunks;         //number of chunks
    size_t last_chunk_size;        //the remaining size if not aligned
    size_t last_chunk_offset;      //where the last chunk starts
    size_t last_chunk_offset_size; //the size size of the last chunk calculated from the offset
    bool aligned;                  //set if size is not divisable by block_size
    block_t *blocks;             //the hashes of each block
} file_t;

file_t *new_file(char *file);
file_t *populate_file_stats(char *file);
void hash_file(file_t *file);

#endif
