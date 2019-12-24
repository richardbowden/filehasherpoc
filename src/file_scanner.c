#include "file_scanner.h"
#include "debug.h"
#include "meowhash.h"
#include <assert.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

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

int FileTypeFile = 0;
int FileTypeDir = 1;

// Globals local to this file @THREADS
static file_fifo_t* file_queue;
static size_t scanned_bytes;
static size_t scanned_files;
static int root_length = 0;

void file_fifo_add(file_fifo_t* list, file_t* file)
{
    assert(file->file_abs[0] == '/');
    if (list->head == NULL) {
        list->head = file;
        list->tail = file;
        list->head->prev = NULL;
        list->head->next = NULL;
        list->tail->next = NULL;
        list->tail->prev = NULL;
    } else {
        list->tail->next = file;
        file->prev = list->tail;
        list->tail = file;
        list->tail->next = NULL;
    }
    list->count++;
}

file_t* new_file(const char* file, const struct stat* f_info)
{
    //    printf("%d - %s\n", file[0], file);
    assert(file[0] == '/');

    int cur_file_len = strlen(file);
    int rel_len = cur_file_len - root_length;
    int rel_start = root_length;
    rel_start++; //add one to skip past the / between the root path and the relative path
    char* rel_path;

    rel_path = malloc(rel_len * sizeof(char)); //+1 to allow space for \0 (null)
    strncpy(rel_path, file + rel_start, rel_len);
    rel_path[rel_len - 1] = '\0'; // ensure null terminated string

    DEBUG_PRINT("rel_path: %s\n", rel_path);

    size_t number_of_blocks = f_info->st_size / BLOCK_SIZE;
    size_t last_block_size = f_info->st_size % BLOCK_SIZE;

    if (last_block_size != 0) {
        number_of_blocks += 1;
    }

    file_t* f;
    f = (file_t*)calloc(1, sizeof(file_t));

    char* my_file_abs = strdup(file);

    f->file_abs = my_file_abs;
    f->file_rel = rel_path;

    f->uid = f_info->st_uid;
    f->gid = f_info->st_gid;

    f->block_size = BLOCK_SIZE;
    f->size = f_info->st_size;

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
    f->atimespec = f_info->st_atimespec; /* time of last access */
    f->mtimespec = f_info->st_mtimespec; /* last data modification */
    f->ctimespec = f_info->st_ctimespec; /* last status change*/
#else
#error add a, m and c time for linux
#endif

    block_t* b = (block_t*)calloc(number_of_blocks, sizeof(block_t));

    f->blocks = b;
    return f;
}

// void populate_file_stats(file_t *file)
// {

//     size_t number_of_blocks = file->f_info.st_size / BLOCK_SIZE;
//     size_t last_block_size = file->f_info.st_size % BLOCK_SIZE;

//     file->block_size = BLOCK_SIZE;
//     file->size = file->f_info.st_size;
//     if (last_block_size == 0)
//     {
//         file->aligned = true;
//         file->block_count = number_of_blocks;
//     }
//     else
//     {
//         file->block_count = number_of_blocks + 1;
//         file->last_block_size = last_block_size;

//         file->last_block_offset = number_of_blocks * BLOCK_SIZE;
//         file->aligned = false;
//         file->below_block = true;
//     }

//     block_t *b = (block_t *)calloc(file->block_count, sizeof(block_t));

//     file->blocks = b;
// }

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

void hash_file(file_t* f)
{

    FILE* fp;
    fp = fopen(f->file_abs, "rb");

    if (fp == NULL) {
        fprintf(stderr, "failed to open file: %s\n", f->file_abs);
        exit(EXIT_FAILURE);
    }

    size_t cur_bytes_read = 0;
    size_t total_read = 0;
    size_t block_counter = 0;
    char unsigned buffer[BLOCK_SIZE]; //on the stack so its fast

    meow_state* ms = (meow_state*)calloc(1, sizeof(meow_state));

    MeowBegin(ms, &MeowDefaultSeed);

    while ((cur_bytes_read = fread(buffer, sizeof(char), BLOCK_SIZE, fp)) > 0) {

        total_read += cur_bytes_read;

        MeowAbsorb(ms, cur_bytes_read, &buffer);
        
        meow_u128 Hash = MeowHash(MeowDefaultSeed, cur_bytes_read, buffer);
        f->blocks[block_counter].mode = BM_BLOCKS | BM_HASH_MEOW;
        f->blocks[block_counter].offset = total_read;

        sprintf(f->blocks[block_counter].str, "%08llX%08llX", MeowU64From(Hash, 1), MeowU64From(Hash, 0));

        f->blocks[block_counter].raw_high = MeowU64From(Hash, 1);
        f->blocks[block_counter].raw_low = MeowU64From(Hash, 0);
        size_t g = sizeof(block_t);
        block_counter += 1;
    }

    meow_u128 whole_file_hash = MeowEnd(ms, NULL);
    
    DEBUG_PRINT("file: %s, whole file hash: %08llX-%08llX\n",
            f->file_rel,
            MeowU64From(whole_file_hash, 1),
            MeowU64From(whole_file_hash, 0));

    sprintf(f->whole_file_hash.str, "%08llX%08llX", MeowU64From(whole_file_hash, 1), MeowU64From(whole_file_hash, 0));
    
    f->whole_file_hash.mode = BM_FILE | BM_HASH_MEOW;
    f->whole_file_hash.offset = 0;
    f->whole_file_hash.raw_high = MeowU64From(whole_file_hash, 1);
    f->whole_file_hash.raw_low = MeowU64From(whole_file_hash, 0);

    f->block_count = block_counter;
    fclose(fp);
    free(ms);
}

file_t* fs_fifo_pop(file_fifo_t* list)
{
    if (list->head == NULL) {
        return NULL;
    }

    file_t* head = list->head;
    if (head->next != NULL) {
        list->head = head->next;
        list->head->prev = NULL;

        head->next = NULL;
        head->prev = NULL;
        list->count--;
    } else {
        list->tail = NULL;
        list->head = NULL;
        list->count = 0;
    }

    return head;
}

file_fifo_t* new_file_fifo()
{
    file_fifo_t* f;
    f = calloc(1, sizeof(file_fifo_t));
    return f;
}

int file_handler(const char* cur_file, const struct stat* f_stat, int i)
{

    file_t* f = NULL;
    switch (i) {
        //                case FTW_d:
        //            f = new_file(cur_file);
        //                    break;
    case FTW_F:
        f = new_file(cur_file, f_stat);
        f->type = FileTypeFile;

        scanned_bytes += f_stat->st_size;
        scanned_files++;
        
        printf("\r\033[K scanned: %zu, bytes: %zu", scanned_files, scanned_bytes);
        fflush(stdout);
        
        time_t s, t;
        s = clock();
        hash_file(f);
        t = clock() - s;
        double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds
        f->hash_scan_time = time_taken;

        file_fifo_add(file_queue, f);
        break;
    default:
        break;
    }

    //    DEBUG_PRINT("scanned_files: %ld\r", scanned_files);
    //    DEBUG_PRINT("scanned files: %ld, scanned bytes: %ld\r", scanned_files, scanned_bytes);

    return 0;
}

int (*file_handle)(const char* f, const struct stat* f_stat, int i) = file_handler;

int fs_get_files(char* root_dir, file_fifo_t* queue)
{
    //    DEBUG_PRINT("root_dir_to_scan: %s\n", root_dir);

    //set the internal queue to the users queue
    file_queue = queue;
    scanned_bytes = 0;
    scanned_files = 0;

    root_length = strlen(root_dir);

    if (root_dir[root_length - 1] == '/') {
        root_dir[root_length - 1] = '\0';
        root_length -= 1;
    }

    int res = ftw(root_dir, file_handle, 32);

    //    DEBUG_PRINT("scanned_files: %ld\r", scanned_files);
    //    DEBUG_PRINT("\n");

    if (res) {
        printf("fucked");
        exit(EXIT_FAILURE);
    }

    return 0;
}
