you need cmake 3.x

run all commands on the root of this repo

setup cmake env

    cmake -H. -Bbuild


build the binary

    cmake --build build -- -j8


run to tool to hash a file

    ./build/filescanner /a/path/to/file.mpg