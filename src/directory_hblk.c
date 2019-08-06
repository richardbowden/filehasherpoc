#include "directory_hblk.h"
#include "directories.h"
#include "string.h"
#include "debug.h"

uint8_t HSYNCSignature[] = {0x89, 0x48, 0x53, 0x59, 0x4E, 0x43, 0x89, 0x86};
char FILEHDRSignature[] = {'F', 'H', 'D', 'R'};

int SyncDirMajor = 0;
int SyncDirMinor = 1;

size_t sync_dir_write_file(char *file, sync_directory *sd)
{
    
    size_t total = 0;
    FILE *h;
    h = fopen(file, "wb");

    // HEADER
    //magic signature
    size_t sig_size = sizeof HSYNCSignature;
    DEBUG_PRINT("sig_size: %zu\n", sig_size);
    fwrite(&HSYNCSignature, sig_size, 1, h);

    //    total += sizeof(SyncDirSignature);

    //version
    size_t ver_maj_size = sizeof SyncDirMajor;
    size_t ver_min_size = sizeof SyncDirMinor;
    DEBUG_PRINT("ver major: %zu\n", ver_maj_size);
    DEBUG_PRINT("ver_minor: %zu\n", ver_min_size);
    
    fwrite(&SyncDirMajor, ver_maj_size, 1, h);
    fwrite(&SyncDirMinor, ver_min_size, 1, h);

    //base path
    size_t root_size = strlen(sd->root)+1;
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

        //abs_path
        s = strlen(sd->files[i]->file_abs)+1;
        fwrite(sd->files[i]->file_abs, s, 1, h);
        //file
        s = strlen(sd->files[i]->file_rel);
        fwrite(sd->files[i]->file_rel, s + 1, 1, h);

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
//        printf ("\n\n\n%ld\n", sizeof(sd->files[i]->atimespec));
//        fwrite(&sd->files[i]->atimespec, sizeof(sd->files[i]->atimespec), 1, h);
//        fwrite(&sd->files[i]->mtimespec, sizeof(sd->files[i]->mtimespec), 1, h);
//        fwrite(&sd->files[i]->ctimespec, sizeof(sd->files[i]->ctimespec), 1, h);
    }

    fclose(h);
    return total;
}
