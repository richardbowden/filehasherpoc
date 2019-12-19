#if !defined(__DATABASE)
#define __DATABASE
#include <sqlite3.h>
#include "directories.h"

void db_init();
int db_add_set(sync_directory *sd, int in_trans);
int db_add_file(file_t *f, int set_id, int in_trans);
void db_add_blocks(file_t *f, int file_id, int in_trans);
void db_close();

int db_sd_import(sync_directory *sd);
int db_hblk_compare(char *primary, char *secondary);


#endif // __DATABASE
