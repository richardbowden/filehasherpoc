#include "database.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "directories.h"
#include "directory_hblk.h"

static sqlite3 *db;

void db_create_tables();

static char *add_set = "insert into sets (set_name, base_path, hostname, file_count, started_at, finished_at) values(?,?,?,?,?,?);";
static sqlite3_stmt *add_set_query;

static char *add_file = "insert into files (set_id, file_abs, file_rel, size, gid, uid, atimespec, mtimespec, ctimespec, block_count, whole_file_hash_id) values(?,?,?,?,?,?,?,?,?,?,?);";
static sqlite3_stmt *add_file_query;

static char *add_blocks = "insert into blocks (raw_high, raw_low, str) values(?,?,?);";
static sqlite3_stmt *add_blocks_query;

static char *add_file_blocks = "insert into file_blocks (file_id, block_id, block_pos, mode, offset) values(?,?,?,?,?);";
static sqlite3_stmt *add_file_blocks_query;

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

    sqlite3_prepare_v2(db, add_set, -1, &add_set_query, 0);
    sqlite3_prepare_v2(db, add_file, -1, &add_file_query, 0);
    sqlite3_prepare_v2(db, add_blocks, -1, &add_blocks_query, 0);
    sqlite3_prepare_v2(db, add_file_blocks, -1, &add_file_blocks_query, 0);
}

void db_create_tables()
{

    char *sets_table = "create table if not exists sets ("
                       "id INTEGER PRIMARY KEY NOT NULL,"
                       "set_name TEXT NOT NULL,"
                       "hostname TEXT NOT NULL,"
                       "started_at TEXT NOT NULL,"
                       "finished_at TEXT NOT NULL,"
                       "base_path TEXT NOT NULL,"
                       "file_count INTEGER NOT NULL"
                       ");";

    char *files_table = "create table if not exists files ("
                        "id INTEGER PRIMARY KEY NOT NULL,"
                        "set_id INTEGER NOT NULL,"
                        "file_abs TEXT NOT NULL,"
                        "file_rel TEXT NOT NULL,"
                        "size INTEGER NOT NULL,"
                        "uid TEXT NOT NULL,"
                        "gid TEXT NOT NULL,"
                        "atimespec TEXT NOT NULL,"
                        "mtimespec TEXT NOT NULL,"
                        "ctimespec TEXT NOT NULL,"
                        "whole_file_hash_id INTEGER NOT NULL,"
                        "block_count INTEGER NOT NULL,"
                        "FOREIGN KEY (set_id) REFERENCES sets (id)"
                        ");";

    char *blocks = "create table if not exists blocks ("
                   "id INTEGER PRIMARY KEY NOT NULL,"
                   "raw_high INTEGER NOT NULL,"
                   "raw_low INTEGER NOT NULL,"
                   "str TEXT NOT NULL"
                   ");";

    char *file_blocks = "create table if not exists file_blocks ("
                    "id INTEGER PRIMARY KEY NOT NULL,"
                    "file_id INTEGER NOT NULL,"
                    "block_id INTEGER NOT NULL,"
                    "block_pos INTEGER NOT NULL,"
                    "mode TEXT NOT NULL,"
                    "offset TEXT NOT NULL,"
                    "FOREIGN KEY (file_id) REFERENCES files (id),"
                    "FOREIGN KEY (block_id) REFERENCES blocks (id)"
                    ");";

    sqlite3_stmt *query;
    const char *tail;
    sqlite3_prepare_v2(db, sets_table, strlen(sets_table), &query, &tail);

    int res = sqlite3_step(query);
    if (res != SQLITE_DONE)
    {
        printf("error creating sets table: %s\n", sqlite3_errstr(res));
        exit(EXIT_FAILURE);
    }
    sqlite3_reset(query);
    
    //files
    sqlite3_prepare_v2(db, files_table, strlen(files_table), &query, &tail);
    res = sqlite3_step(query);
    if (res != SQLITE_DONE)
    {
        printf("error creating files table: %s\n", sqlite3_errstr(res));
        exit(EXIT_FAILURE);
    }
    sqlite3_reset(query);
    
    
    //blocks
    sqlite3_prepare_v2(db, blocks, strlen(blocks), &query, &tail);
    res = sqlite3_step(query);
    if (res != SQLITE_DONE)
    {
        printf("error creating blocks table: %s\n", sqlite3_errstr(res));
        exit(EXIT_FAILURE);
    }
    sqlite3_reset(query);
    
    sqlite3_prepare_v2(db, file_blocks, strlen(file_blocks), &query, &tail);
    res = sqlite3_step(query);
    if (res != SQLITE_DONE)
    {
        printf("error creating file blocks table: %s\n", sqlite3_errstr(res));
        exit(EXIT_FAILURE);
    }
    sqlite3_finalize(query);
}

int db_add_set(sync_directory *sd, int in_trans)
{
    sqlite3_reset(add_set_query);
//    char *add_set = "insert into sets (set_name, base_path, hostname, file_count, started_at, finished_at) values(?,?,?,?,?,?);";

//    sqlite3_stmt *query;
//    sqlite3_prepare_v2(db, add_set, -1, &query, 0);
    sqlite3_bind_text(add_set_query, 1, sd->set_name, -1, NULL);
    sqlite3_bind_text(add_set_query, 2, sd->root, -1, NULL);
    sqlite3_bind_text(add_set_query, 3, sd->hostname, -1, NULL);
    sqlite3_bind_int(add_set_query, 4, sd->files_count);
    sqlite3_bind_int(add_set_query, 5, sd->started_at);
    sqlite3_bind_int(add_set_query, 6, sd->finished_at);

    int res = sqlite3_step(add_set_query);
    if (res != SQLITE_DONE)
    {
        fprintf(stderr, "database error: %s\n", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    
    if (!in_trans){
        sqlite3_finalize(add_set_query);
    }
    
    int id = sqlite3_last_insert_rowid(db);
//    sqlite3_finalize(query);
    return id;
}

int db_add_file(file_t *f, int set_id, int in_trans){
//    char *add_file = "insert into files (set_id, file_abs, file_rel, size, gid, uid, atimespec, mtimespec, ctimespec, block_count, whole_file_hash_id) values(?,?,?,?,?,?,?,?,?,?,?);";
    sqlite3_reset(add_file_query);
    
    int whole_file_id;
    db_add_block(&f->whole_file_hash, 1,  &whole_file_id);
    
//    sqlite3_stmt *query;
//    sqlite3_prepare_v2(db, add_file, -1, &query, 0);
    sqlite3_bind_int(add_file_query, 1, set_id);
    sqlite3_bind_text(add_file_query, 2, f->file_abs, -1, NULL);
    sqlite3_bind_text(add_file_query, 3, f->file_rel, -1, NULL);
    sqlite3_bind_int(add_file_query, 4, f->size);
    sqlite3_bind_int(add_file_query, 5, f->gid);
    sqlite3_bind_int(add_file_query, 6, f->uid);
    sqlite3_bind_int(add_file_query, 7, f->atimespec.tv_nsec);
    sqlite3_bind_int(add_file_query, 8, f->mtimespec.tv_nsec);
    sqlite3_bind_int(add_file_query, 9, f->ctimespec.tv_nsec);
    sqlite3_bind_int(add_file_query, 10, f->block_count);
    sqlite3_bind_int(add_file_query, 11, whole_file_id);
    
    int res = sqlite3_step(add_file_query);
    if (res != SQLITE_DONE){
        fprintf(stderr, "database error: %s\n", sqlite3_errmsg(db));
        if (in_trans){
            sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        }
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    
    if (!in_trans){
        sqlite3_finalize(add_file_query);
    }
    
    int id = sqlite3_last_insert_rowid(db);
//    sqlite3_finalize(query);
    return id;
}

void db_add_block(block_t *block, int in_trans, int *b_id){
    sqlite3_reset(add_blocks_query);
    sqlite3_clear_bindings(add_blocks_query);


    sqlite3_bind_int64(add_blocks_query, 1, block->raw_high);
    sqlite3_bind_int64(add_blocks_query, 2, block->raw_low);
    sqlite3_bind_text(add_blocks_query, 3, block->str, -1 ,NULL);

    int res = sqlite3_step(add_blocks_query);
        
    if (res != SQLITE_DONE){
        fprintf(stderr, "db_error_add_block: %s\n", sqlite3_errmsg(db));
        if (in_trans){
            sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        }
        
        sqlite3_close(db);
        exit(EXIT_FAILURE);
        
    }

    if (!in_trans){
        sqlite3_finalize(add_blocks_query);
    }
    
    *b_id = sqlite3_last_insert_rowid(db);
}

void db_close(){
    sqlite3_close(db);
}

void db_add_file_block(int file_id, int block_id, int mode, int pos, size_t offset){

//    char *add_file_blocks = "insert into file_blocks (file_id, block_id, block_pos, mode, offset) values(?,?,?,?,?);";
//    sqlite3_stmt *query;
//    sqlite3_prepare_v2(db, add_file_blocks, -1, &query, 0);
    sqlite3_reset(add_file_blocks_query);
    sqlite3_bind_int(add_file_blocks_query, 1, file_id);
    sqlite3_bind_int(add_file_blocks_query, 2, block_id);
    sqlite3_bind_int(add_file_blocks_query, 3, pos);
    sqlite3_bind_int(add_file_blocks_query, 4, mode);
    sqlite3_bind_int(add_file_blocks_query, 5, offset);
    
    int res = sqlite3_step(add_file_blocks_query);
    
    if (res != SQLITE_DONE){
        fprintf(stderr, "db_error_add_file_block: %s\n", sqlite3_errmsg(db));

        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    
//    sqlite3_finalize(query);
    
}

int db_sd_import(sync_directory *sd){
    //TODO: need to make an itarator for hblf files
    // sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    int set_id = db_add_set(sd, true);
    int commit_counter = 0;
    for (size_t i = 0; i < sd->files_count; i++) {
        if (commit_counter == 0){
            sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
        }
        int id = db_add_file(sd->files[i], set_id, true);
        int b_id;
        for (int b = 0; b < sd->files[i]->block_count; b++) {
            db_add_block(&sd->files[i]->blocks[b], true, &b_id);
            db_add_file_block(id, b_id, sd->files[i]->blocks[b].mode, b+1, sd->files[i]->blocks[b].offset);
        }
        
//        db_add_block(sd->files[i]->blocks,sd->files[i]->block_count, id, true, NULL);
//        printf("\r\033[K %lu - %s", i, sd->files[i]->file_rel);
        printf("\r\033[K %lu", i);
        fflush(stdout);
        commit_counter ++;
        if (commit_counter == 10000){
            sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
//            printf("commiting db\n");
            commit_counter = 0;
        }
    }
    
//    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    
//    sql
    
    sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
    
    return 0;
};
