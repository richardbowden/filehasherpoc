#include "database.h"
#include <stdlib.h>
#include <stdio.h>
#include <emmintrin.h>
#include <x86intrin.h>
#include "meowhash.h"

int main(){
    db_init();

    char *q = "select raw from blocks where id = 1;";

    sqlite3_stmt *query;
    sqlite3_prepare_v2(db, q, -1, &query, 0);

    int res = sqlite3_step(query);

//    if (res != SQLITE_DONE)
//    {
//        printf("error creating sets table: %s\n", sqlite3_errstr(res));
//        exit(EXIT_FAILURE);
//    }
//    const void *f;
    __m128i f = _mm_setzero_si128();


    const void *s;
    if (res == SQLITE_ROW){

        int size = sqlite3_column_bytes(query, 0);
        _mm_storeu_si128(&f,sqlite3_column_blob(query, 0));
//        __m128i ss = (__m128i) sqlite3_column_blob(query, 0);
    }

    return EXIT_SUCCESS;
}