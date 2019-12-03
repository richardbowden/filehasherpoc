#include "database.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "directories.h"

static sqlite3 *db;
void db_create_tables();


void db_init(){
    int rc;
    rc = sqlite3_open("filecompare", &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
       }
    db_create_tables();
}

void db_create_tables(){
    
    char *file_table = "create table if not exists set (setid INTEGER PRIMARY KEY, set_name TEXT, base_path TEXT, hostname TEXT, file_count INTEGER);";
    sqlite3_stmt *query;
    const char *tail;
    sqlite3_prepare_v2(db, file_table, strlen(file_table), &query, &tail);
    
    int res = sqlite3_step(query);
    printf("error: %s\n", sqlite3_errstr(res));
    
//    insert into files (set_name, base_path, file_count) values("default", "/root", 100);
}

void db_add_set(sync_directory *sd){
    char *add_set = "insert into files (set_name, base_path, hostname, file_count) values(?,?,?,?);";
    
    sqlite3_stmt *query;
    sqlite3_prepare_v2(db, add_set, -1, &query, 0);
    sqlite3_bind_text(query, 1, sd->set_name, -1, NULL);
    sqlite3_bind_text(query, 2, sd->root, -1, NULL);
    sqlite3_bind_text(query, 3, sd->hostname, -1, NULL);
    sqlite3_bind_int(query, 4, sd->files_count);
    
    int res = sqlite3_step(query);
    if (res){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    sqlite3_finalize(query);

}
