#include "directory_hblk.h"
#include "directories.h"
#include "string.h"
#include "debug.h"
#include <errno.h>
#include "file_scanner.h"
#include <assert.h>

int errno;

uint8_t HSYNCSignature[] = {0x89, 0x48, 0x53, 0x59, 0x4E, 0x43, 0x89, 0x86};
uint8_t FILEHDRSignature[] = {0x89, 'F', 'H', 'D', 'R', 0x89, 0x86, 0x86};

int SyncDirMajor = 0;
int SyncDirMinor = 1;

void sync_dir_read_file(char *file, sync_directory *sd)
{
    size_t total = 0;
    FILE *h;

    int errnum = 0;

    h = fopen(file, "r");
    if (h == NULL)
    {
        errnum = errno;
        printf("Error (%d), msg: %s - %s\n", errnum, file, strerror(errnum));
        exit(EXIT_FAILURE);
    }

    //read magic value
    uint8_t file_magic[8];
    fread(&file_magic, sizeof(HSYNCSignature), 1, h);

    int r = ((int)memcmp(&file_magic[0], &HSYNCSignature[0], 8));

    if (r){
        printf("not a valid hblk file\n");
        exit(EXIT_FAILURE);
    }else{
        printf("valid hblk file\n\n");
    }
    
    //hostname
    size_t hostname_size;
    fread(&hostname_size, sizeof(size_t), 1, h);

    char *hostname = malloc(hostname_size);
    fread(hostname, hostname_size, 1, h);

    //set_name
    size_t set_name_size;
    fread(&set_name_size, sizeof(size_t), 1, h);

    char *set_name = malloc(set_name_size);
    fread(set_name, set_name_size, 1, h);
    
    int major;
    fread(&major, sizeof(int), 1, h);

    int minor;
    fread(&minor, sizeof(int), 1, h);

    //timestamps
    time_t started_at;
    fread(&started_at, sizeof(time_t), 1, h);
    
    time_t finished_at;
    fread(&finished_at, sizeof(time_t), 1, h);
    
    size_t base_path_len;
    fread(&base_path_len, sizeof(size_t), 1, h);

    char *base_path = malloc(base_path_len);
    fread(base_path, base_path_len, 1, h);

    size_t file_count;
    fread(&file_count, sizeof(size_t), 1, h);

//    sync_directory *sync_dir_from_file = sync_dir_new(base_path, file_count, 0);



    printf("hostname:   %s\n", hostname);
    printf("set_name:   %s\n", set_name);
    printf("base_path:  %s\n", base_path);
    printf("file_count: %zu\n", file_count);
    printf("started_at: %zu\n", started_at);
    printf("finished_at: %zu\n", finished_at);
    
    int blk_counter = 0;
    int file_counter = 0;
    
    for (int i = 0; i < file_count; i++)
    {
        uint8_t ss[8];
        fread(&ss, 1, 8, h);

        int res = ((int)memcmp(&ss[0], &FILEHDRSignature[0], 8));

        if (res != 0)
        {
            exit(-2);
        }

        size_t path_abs_len;
        fread(&path_abs_len, sizeof(size_t), 1, h);
        char *path_abs = malloc(sizeof(char) * path_abs_len);
        fread(path_abs, sizeof(char), path_abs_len, h);

        size_t path_rel_len;
        fread(&path_rel_len, sizeof(size_t), 1, h);
        char *path_rel = malloc(sizeof(char) * path_rel_len);
        fread(path_rel, sizeof(char), path_rel_len, h);

        int type;
        fread(&type, sizeof(int), 1, h);

        int block_size = 0;
        fread(&block_size, sizeof(int), 1, h);

        size_t file_size = 0;
        fread(&file_size, sizeof(size_t), 1, h);

        int uid;
        fread(&uid, sizeof(int), 1, h);

        int gid;
        fread(&gid, sizeof(int), 1, h);

        size_t timespec_s = sizeof(struct timespec);
        struct timespec *atime = calloc(1, timespec_s);
        int result = fread(atime, timespec_s, 1, h);

        struct timespec *mtime = calloc(1, timespec_s);
        result = fread(mtime, timespec_s, 1, h);

        struct timespec *ctime = calloc(1, timespec_s);
        result = fread(ctime, timespec_s, 1, h);

        size_t block_count;
        fread(&block_count, sizeof(size_t), 1, h);

        block_t *b = calloc(block_count, sizeof(block_t));

        fread(b, sizeof(block_t), block_count, h);
        
#if 0
        for (int bi = 0; bi < block_count; bi++)
        {

            printf("%s, %d, ", path_abs, bi);
            printf("%d, %d, %d, %d, ",
                   b[bi].hash[3],
                   b[bi].hash[2],
                   b[bi].hash[1],
                   b[bi].hash[0]);

            printf("%08X-%08X-%08X-%08X\n",
                   b[bi].hash[3],
                   b[bi].hash[2],
                   b[bi].hash[1],
                   b[bi].hash[0]);
        }
#endif
        file_counter++;
    }

    if (!(file_counter == file_count)){
        printf("read failed, count miss match expected: %zu, got: %d\n", file_count, file_counter);
    }else{
        printf("total_found: %d\n", file_counter);
    }
    
    fclose(h);
}

size_t sync_dir_write_file(char *file, sync_directory *sd)
{

    size_t total = 0;
    FILE *h;
    h = fopen(file, "w");

    // HEADER
    //magic signature
    size_t sig_size = sizeof HSYNCSignature;
    DEBUG_PRINT("sig_size: %zu\n", sig_size);
    fwrite(&HSYNCSignature, sig_size, 1, h);

    //    total += sizeof(SyncDirSignature);

    //hostname
    size_t hostname_size = strlen(sd->hostname)+1;
    fwrite(&hostname_size, sizeof(hostname_size), 1, h);
    fwrite(sd->hostname, hostname_size, 1, h);

    //set_name
    size_t set_name_size = strlen(sd->set_name)+1;
    fwrite(&set_name_size, sizeof(set_name_size), 1, h);
    fwrite(sd->set_name, set_name_size, 1, h);
    
    //version
    size_t ver_maj_size = sizeof SyncDirMajor;
    size_t ver_min_size = sizeof SyncDirMinor;
    DEBUG_PRINT("ver major: %zu\n", ver_maj_size);
    DEBUG_PRINT("ver_minor: %zu\n", ver_min_size);

    fwrite(&SyncDirMajor, ver_maj_size, 1, h);
    fwrite(&SyncDirMinor, ver_min_size, 1, h);

    //timestamps
    size_t started_at_size = sizeof(sd->started_at);
    fwrite(&sd->started_at, started_at_size, 1, h);
    
    size_t finished_at_size = sizeof(sd->finished_at);
    fwrite(&sd->finished_at, finished_at_size, 1, h);
    
    //base path
    size_t root_size = strlen(sd->root) + 1;
    fwrite(&root_size, sizeof(root_size), 1, h);
    DEBUG_PRINT("root size: %zu\n", root_size);
    fwrite(sd->root, root_size, 1, h);

    //number of files
    size_t files_size = sizeof(sd->files_count);
    DEBUG_PRINT("file_size: %zu\n", files_size);
    fwrite(&sd->files_count, files_size, 1, h);

    //END OF HEADER

    //Files array
    for (int i = 0; i < sd->files_count; i++)
    {
        size_t s;
        fwrite(&FILEHDRSignature, sizeof(FILEHDRSignature), 1, h);

        //path_abs
        assert(sd->files[i]->file_abs[0] == '/');
        s = strlen(sd->files[i]->file_abs) + 1;

        fwrite(&s, sizeof(size_t), 1, h);
        fwrite(sd->files[i]->file_abs, s, 1, h);

        //path_rel
        s = strlen(sd->files[i]->file_rel) + 1;
        fwrite(&s, sizeof(size_t), 1, h);
        fwrite(sd->files[i]->file_rel, s, 1, h);

        //type
        fwrite(&sd->files[i]->type, sizeof(sd->files[i]->type), 1, h);

        //block_size
        fwrite(&sd->files[i]->block_size, sizeof(sd->files[i]->type), 1, h);

        //file_size
        fwrite(&sd->files[i]->size, sizeof(sd->files[i]->size), 1, h);

        //user id
        fwrite(&sd->files[i]->uid, sizeof(sd->files[i]->uid), 1, h);

        //group id
        fwrite(&sd->files[i]->gid, sizeof(sd->files[i]->gid), 1, h);

        //a,m and ctime
        fwrite(&sd->files[i]->atimespec, sizeof(sd->files[i]->atimespec), 1, h);
        fwrite(&sd->files[i]->mtimespec, sizeof(sd->files[i]->mtimespec), 1, h);
        fwrite(&sd->files[i]->ctimespec, sizeof(sd->files[i]->ctimespec), 1, h);

        // add blocks

        fwrite(&sd->files[i]->block_count, sizeof(sd->files[i]->block_count), 1, h);

        int cbc = sd->files[i]->block_count;

        size_t hsize = sizeof(sd->files[i]->blocks[0]);

        for (int bi = 0; bi < cbc; bi++)
        {

            fwrite(&sd->files[i]->blocks[bi], hsize, 1, h);
#if 0
            printf("%s, %d, ", sd->files[i]->file_abs, bi);
            printf("%d, %d, %d, %d, ",
                   sd->files[i]->blocks[bi].hash[3],
                   sd->files[i]->blocks[bi].hash[2],
                   sd->files[i]->blocks[bi].hash[1],
                   sd->files[i]->blocks[bi].hash[0]
                   );
            printf("%08X-%08X-%08X-%08X\n",
                   sd->files[i]->blocks[bi].hash[3],
                   sd->files[i]->blocks[bi].hash[2],
                   sd->files[i]->blocks[bi].hash[1],
                   sd->files[i]->blocks[bi].hash[0]
                   );
#endif
        }
    }

    fclose(h);
    return total;
}
