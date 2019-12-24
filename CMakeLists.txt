cmake_minimum_required(VERSION 3.14)
project(filescanner LANGUAGES C)

set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "1")
set(PROJECT_VERSION "0.0.1")

#MESSAGE(STATUS "CMAKE_BINARY_DIR " ${CMAKE_BINARY_DIR})
#MESSAGE(STATUS "PROJECT_SOURCE_DIR " ${PROJECT_SOURCE_DIR}/src)
#MESSAGE(STATUS "CMAKE_SOURCE_DIR " ${CMAKE_SOURCE_DIR})


#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)
message (STATUS "build type: " ${CMAKE_BUILD_TYPE})

INCLUDE(CheckCCompilerFlag)

find_package(SQLite3 REQUIRED)

add_executable(filescanner 
	src/main.c 
	src/file_scanner.c 
	src/file_scanner.h 
	src/directories.c 
	src/directories.h 
	src/debug.h 
	src/directory_hblk.c 
	src/directory_hblk.h
	src/database.h
	src/database.c
		)

target_compile_options(filescanner PUBLIC -maes -mavx2)

#add_executable(dd src/dbtest.c src/database.c src/database.h src/meowhash.h)

target_link_libraries(filescanner ${SQLite3_LIBRARIES})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#target_link_libraries(dd ${SQLite3_LIBRARIES})
