#include "file_scanner.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <ftw.h>
#include "meowhash.h"

#define KILOBYTE 1024
#define MEGABYTE (KILOBYTE * KILOBYTE)
#define GIGABYTE (MEGABYTE * KILOBYTE)

// Data to create dynamic block sizes depending on the size of the file
//Block Sizes
//---------------------------------
// File Size	        Block Size
// 0 - 250 MiB	        128 KiB
// 250 MiB - 500 MiB	256 KiB
// 500 MiB - 1 GiB	    512 KiB
// 1 GiB - 2 GiB	    1 MiB
// 2 GiB - 4 GiB	    2 MiB
// 4 GiB - 8 GiB	    4 MiB
// 8 GiB - 16 GiB	    8 MiB
// 16 GiB - up	        16 MiB
size_t BLOCK_SIZE = 256 * KILOBYTE;

file_fifo_t *file_queue;

void file_fifo_add(file_fifo_t *list, file_t *file)
{

    if (list->head == NULL)
    {
        list->head = file;
        list->tail = file;
        list->head->prev = NULL;
        list->head->next = NULL;
        list->tail->next = NULL;
        list->tail->prev = NULL;
    }
    else
    {
        list->tail->next = file;
        file->prev = list->tail;
        list->tail = file;
        list->tail->next = NULL;
    }
}

file_t *new_file(const char *file)
{
    file_t *f;
    f = calloc(1, sizeof(file_t));
    f->file = strdup(file);
    return f;
}

void populate_file_stats(file_t *file)
{

    size_t number_of_blocks = file->f_info.st_size / BLOCK_SIZE;
    size_t last_block_size = file->f_info.st_size % BLOCK_SIZE;

    file->block_size = BLOCK_SIZE;
    file->size = file->f_info.st_size;
    if (last_block_size == 0)
    {
        file->aligned = true;
        file->block_count = number_of_blocks;
    }
    else
    {
        file->block_count = number_of_blocks + 1;
        file->last_block_size = last_block_size;

        file->last_block_offset = number_of_blocks * BLOCK_SIZE;
        file->aligned = false;
        file->below_block = true;
    }

    block_t *b = (block_t *)calloc(file->block_count, sizeof(block_t));

    file->blocks = b;
}

#if false
void readable_fs(double size /*in bytes*/, char **buf)
{
    int i = 0;
    const char *units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1024)
    {
        size /= 1024;
        i++;
    }

    size_t s = sizeof(i) + sizeof(size) + sizeof(units[i]) + 1;
    *buf = (char *)malloc(s);
    sprintf(*buf, "%.02lf %s", size, units[i]);
}
#endif

void hash_file(file_t *f)
{
    FILE *fp;
    fp = fopen(f->file, "rb");

    if (fp == NULL)
    {
        fprintf(stderr, "failed to open file: %s\n", f->file);
        exit(EXIT_FAILURE);
    }

    size_t cur_bytes_read = 0;
    size_t total_read = 0;
    size_t block_counter = 0;
    char buffer[BLOCK_SIZE]; //on the stack so its fast

    while ((cur_bytes_read = fread(buffer, sizeof(char), BLOCK_SIZE, fp)) > 0)
    {

        total_read += cur_bytes_read;

        meow_u128 Hash = MeowHash(MeowDefaultSeed, cur_bytes_read, buffer);
        f->blocks[block_counter].offset = total_read;
        f->blocks[block_counter].hash[3] = MeowU32From(Hash, 3);
        f->blocks[block_counter].hash[2] = MeowU32From(Hash, 2);
        f->blocks[block_counter].hash[1] = MeowU32From(Hash, 1);
        f->blocks[block_counter].hash[0] = MeowU32From(Hash, 0);

        printf("file: %s, block: %zu, total_bytes: %zu, current_bytes: %zu\n", f->file, block_counter, total_read, cur_bytes_read);

        block_counter += 1;
    }

    fclose(fp);
}

file_fifo_t *new_file_fifo()
{
    file_fifo_t *f;
    f = calloc(1, sizeof(file_fifo_t));
    return f;
}

int file_handler(const char *cur_file, const struct stat *f_stat, int i)
{
    file_t *f;
    switch (i)
    {

    case FTW_F:

        f = new_file(cur_file);
        f->f_info = *f_stat;
        populate_file_stats(f);
        hash_file(f);
        file_fifo_add(file_queue, f);
        break;
    default:
        break;
    }

    return 0;
}

int (*file_handle)(const char *f, const struct stat *f_stat, int i) = file_handler;

// file_fifo_t *scan_files(char *root)
// {
//     file_fifo_t *queue
// }

int fs_get_files(char *root_dir, file_fifo_t *queue)
{

    //set the internal queue to the users queue
    file_queue = queue;

    int res = ftw(root_dir, file_handle, 32);

    if (res)
    {
        printf("fucked");
        exit(EXIT_FAILURE);
    }
    printf("test\n");
    return 0;
}
