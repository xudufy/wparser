cmake_minimum_required(VERSION 3.6)

project(wparser)

add_compile_options(-std=c++17 -O2 -DNDEBUG -Wall -Werror)

add_library(wparser 
    STATIC 
    src/parser.cpp
    src/codegen.cpp
    src/LEB128.cpp
)
file(GLOB_RECURSE wparser_headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
set_target_properties(wparser PROPERTIES PUBLIC_HEADER "${wparser_headers}")
add_subdirectory(src/tools)
add_subdirectory(test)

install(TARGETS wparser ARCHIVE PUBLIC_HEADER)