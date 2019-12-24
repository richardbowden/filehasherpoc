you need cmake 3.x

run all commands on the root of this repo

setup cmake env

    cmake -H. -Bbuild


build the binary

    cmake --build build -- -j8


run to tool to hash a file

    ./build/filescanner /a/path/to/file.mpg



editor things - vim 

sudo port install cmake cquery cscope direnv git-flow


# SQL

Get block info for a file

    select f.file_id, f.file_abs, b.block_pos, b.block0, b.block1, b.block2, b.block3 from blocks as b inner join files as f on b.file_id = f.file_id where f.file_id = 1 order by b.block_pos desc;


https://dbdiagram.io/d

https://app.sqldbm.com/PostgreSQL/Draft/


# load 128bit values
        #include <emmintrin.h>
        #include <x86intrin.h>

        __m128i j;
        j = _mm_set_epi64x(f->blocks[block_counter].raw_high,f->blocks[block_counter].raw_low);