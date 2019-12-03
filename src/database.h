#if !defined(__DATABASE)
#define __DATABASE
#include <sqlite3.h>
#include "directories.h"

void db_init();
void db_add_set(sync_directory *sd);
#endif // __DATABASE
