#include "database.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

static sqlite3 *db;

void db_init(){
    int rc;
    rc = sqlite3_open("filecompare", &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
       }
}


