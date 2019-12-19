#include "database.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "directories.h"
#include "directory_hblk.h"

static sqlite3 *db;
void db_create_tables();

int callback(void *NotUsed, int argc, char **argv,
             char **azColName)
{

    NotUsed = 0;

    for (int i = 0; i < argc; i++)
    {

        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");

    return 0;
}

void db_init()
{
    int rc;
    rc = sqlite3_open("filecompare", &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    char *err;
    int res = sqlite3_exec(db, "PRAGMA foreign_keys = true;", NULL, NULL, &err);

    if (err)
    {
        printf("error: %s\n", sqlite3_errstr(res));
        exit(EXIT_FAILURE);
    }

    db_create_tables();
}

void db_create_tables()
{

    char *sets_table = "create table if not exists sets (set_id INTEGER PRIMARY KEY,"
                       "set_name TEXT NOT NULL,"
                       "hostname TEXT NOT NULL,"
                       "started_at TEXT NOT NULL,"
                       "finished_at TEXT NOT NULL,"
                       "base_path TEXT NOT NULL,"
                       "file_count INTEGER NOT NULL"
                       ");";

    char *files_table = "create table if not exists files (file_id INTEGER PRIMARY KEY,"
                        "set_id INTEGER NOT NULL,"
                        "file_abs TEXT NOT NULL,"
                        "file_rel TEXT NOT NULL,"
                        "size INTEGER NOT NULL,"
                        "uid TEXT NOT NULL,"
                        "gid TEXT NOT NULL,"
                        "atimespec TEXT NOT NULL,"
                        "mtimespec TEXT NOT NULL,"
                        "ctimespec TEXT NOT NULL,"
                        "block_count INTEGER NOT NULL,"
                        "FOREIGN KEY (set_id) REFERENCES sets (set_id)"
                        ");";

    char *blocks = "create table if not exists blocks (block_id INTEGER PRIMARY KEY,"
                   "file_id INTEGER NOT NULL,"
                   "block_pos TEXT NOT NULL,"
                   "block3 TEXT NOT NULL,"
                   "block2 TEXT NOT NULL,"
                   "block1 TEXT NOT NULL,"
                   "block0 TEXT NOT NULL,"
                   "offset TEXT NOT NULL,"
                   "FOREIGN KEY (file_id) REFERENCES files (file_id)"
                   ");";

    sqlite3_stmt *query;
    const char *tail;
    sqlite3_prepare_v2(db, sets_table, strlen(sets_table), &query, &tail);

    int res = sqlite3_step(query);
    if (res != SQLITE_DONE)
    {
        printf("error: %s\n", sqlite3_errstr(res));
        exit(EXIT_FAILURE);
    }

    //files
    sqlite3_prepare_v2(db, files_table, strlen(files_table), &query, &tail);
    res = sqlite3_step(query);
    if (res != SQLITE_DONE)
    {
        printf("error: %s\n", sqlite3_errstr(res));
        exit(EXIT_FAILURE);
    }

    //blocks
    sqlite3_prepare_v2(db, blocks, strlen(blocks), &query, &tail);
    res = sqlite3_step(query);
    if (res != SQLITE_DONE)
    {
        printf("error: %s\n", sqlite3_errstr(res));
        exit(EXIT_FAILURE);
    }
}

int db_add_set(sync_directory *sd, int in_trans)
{
    char *add_set = "insert into sets (set_name, base_path, hostname, file_count, started_at, finished_at) values(?,?,?,?,?,?);";

    sqlite3_stmt *query;
    sqlite3_prepare_v2(db, add_set, -1, &query, 0);
    sqlite3_bind_text(query, 1, sd->set_name, -1, NULL);
    sqlite3_bind_text(query, 2, sd->root, -1, NULL);
    sqlite3_bind_text(query, 3, sd->hostname, -1, NULL);
    sqlite3_bind_int(query, 4, sd->files_count);
    sqlite3_bind_int(query, 5, sd->started_at);
    sqlite3_bind_int(query, 6, sd->finished_at);

    int res = sqlite3_step(query);
    if (res != SQLITE_DONE)
    {
        fprintf(stderr, "database error: %s\n", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    
    if (!in_trans){
        sqlite3_finalize(query);
    }
    
    int id = sqlite3_last_insert_rowid(db);
    return id;
}

int db_add_file(file_t *f, int set_id, int in_trans){
    char *add_file = "insert into files (set_id, file_abs, file_rel, size, gid, uid, atimespec, mtimespec, ctimespec, block_count) values(?,?,?,?,?,?,?,?,?,?);";
    
    sqlite3_stmt *query;
    sqlite3_prepare_v2(db, add_file, -1, &query, 0);
    sqlite3_bind_int(query, 1, set_id);
    sqlite3_bind_text(query, 2, f->file_abs, -1, NULL);
    sqlite3_bind_text(query, 3, f->file_rel, -1, NULL);
    sqlite3_bind_int(query, 4, f->size);
    sqlite3_bind_int(query, 5, f->gid);
    sqlite3_bind_int(query, 6, f->uid);
    sqlite3_bind_int(query, 7, f->atimespec.tv_nsec);
    sqlite3_bind_int(query, 8, f->mtimespec.tv_nsec);
    sqlite3_bind_int(query, 9, f->ctimespec.tv_nsec);
    sqlite3_bind_int(query, 10, f->block_count);
    
    int res = sqlite3_step(query);
    if (res != SQLITE_DONE){
        fprintf(stderr, "database error: %s\n", sqlite3_errmsg(db));
        if (in_trans){
            sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        }
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    
    if (!in_trans){
        sqlite3_finalize(query);
    }
    
    int id = sqlite3_last_insert_rowid(db);
    return id;
}

void db_add_blocks(file_t *f, int file_id, int in_trans){
    char *add_blocks = "insert into blocks (file_id, block_pos, block3, block2, block1, block0, offset) values(?,?,?,?,?,?,?);";
    
    sqlite3_stmt *query;
    sqlite3_prepare_v2(db, add_blocks, -1, &query, 0);
    
    size_t count = f->block_count;
    
    for (int i = 0; i<count; i++) {
        block_t b = f->blocks[i];
        
        sqlite3_bind_int(query, 1, file_id);
        sqlite3_bind_int(query, 2, i+1); //block posistion
        sqlite3_bind_int(query, 3, b.hash[3]);
        sqlite3_bind_int(query, 4, b.hash[2]);
        sqlite3_bind_int(query, 5, b.hash[1]);
        sqlite3_bind_int(query, 6, b.hash[0]);
        sqlite3_bind_int(query, 7, b.offset);
        
        int res = sqlite3_step(query);
        
        if (res != SQLITE_DONE){
            fprintf(stderr, "database error: %s\n", sqlite3_errmsg(db));
            if (in_trans){
                sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
            }
            sqlite3_close(db);
            exit(EXIT_FAILURE);
        }
        sqlite3_reset(query);
    }
    
    if (!in_trans){
        sqlite3_finalize(query);
    }
}

void db_close(){
    sqlite3_close(db);
}


int db_sd_import(sync_directory *sd){
    //TODO: need to make an itarator for hblf files
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    int set_id = db_add_set(sd, true);
    for (size_t i = 0; i < sd->files_count; i++) {
        int id = db_add_file(sd->files[i], set_id, true);
        db_add_blocks(sd->files[i], id, true);
        printf("%s\n", sd->files[i]->file_abs);
        printf("\rfile of X is: %lu", i);
        fflush(stdout);
    }
    
//    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
//    sql
    return 0;
};
