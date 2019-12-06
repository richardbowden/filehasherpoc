#if !defined(__DATABASE)
#define __DATABASE
#include <sqlite3.h>
#include "directories.h"

void db_init();
int db_add_set(sync_directory *sd);
void db_add_file(file_t *f, int set_id);
void db_close();
#endif // __DATABASE
