link_libraries(wparser)

add_executable(function_addresser 
    func_addresser.cpp
)

add_executable(codegen_test 
    codegen_test.cpp
)

install(TARGETS function_addresser RUNTIME)
install(TARGETS codegen_test RUNTIME)