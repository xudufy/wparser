cmake_minimum_required(VERSION 3.6)

project(wparser)

add_compile_options(-std=c++17 -O2 -DNDEBUG -Wall -Werror)

add_library(wparser 
    STATIC 
    src/wparser.cpp
    src/codegen.cpp
    src/utils/LEB128.cpp 
)

add_executable(function_addresser 
    test/func_addresser.cpp
)
target_link_libraries(function_addresser wparser)

add_executable(codegen_test 
    test/codegen_test.cpp
)
target_link_libraries(codegen_test wparser)