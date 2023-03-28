#pragma once
#include <cstdio>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <vector>
#include <map>
#include <unordered_map>

#define W_LOG(format, ...) fprintf(stderr, "[%s:%d]" format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);