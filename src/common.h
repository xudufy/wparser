#pragma once
#include <cstdio>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <iostream>

//use stderr to ensure flush.
#define W_LOG(format, ...) fprintf(stderr, "[%s:%d]" format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define W_LOGVAR(symbol) std::cerr<<#symbol<<":"<<symbol<<std::endl;
namespace wparser {
    using byte = uint8_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using s32 = int32_t;
    using s64 = int64_t;
    using Buffer = std::string;
}