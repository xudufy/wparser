link_libraries(wparser)

add_executable(wasm-pc2func
    wasm_pc2func.cpp
)

install(TARGETS wasm-pc2func RUNTIME)