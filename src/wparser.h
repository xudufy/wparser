#pragma once
#include "common.h"
#include "symbols.h"
#include "utils/LEB128.h"

namespace wparser{

typedef std::string Buffer;

class oBufferStream {
protected:
    uint8_t* buffer = nullptr;
public:
    size_t buffer_size = 0;
    size_t cur_pos = 0;
    
    void openFile(std::string path);
    void openBuffer(Buffer b);
    
    const uint8_t * data() {
        return buffer;
    }

    bool eof() {
        return cur_pos >= buffer_size;
    }

    void close() {
        if (buffer != nullptr) {
            free(buffer);
            buffer = nullptr;
        }
    }

    uint64_t consumeUInt() {
        unsigned int length;
        const char *error = nullptr;
        auto ret = decodeULEB128(buffer + cur_pos, &length, buffer + buffer_size, &error);
        if (error != nullptr) {
            throw std::runtime_error("bad value");
        }
        cur_pos += length;
        return ret;
    }

    uint64_t consumeSInt() {
        unsigned int length;
        const char *error = nullptr;
        auto ret = decodeSLEB128(buffer + cur_pos, &length, buffer + buffer_size, &error);
        if (error != nullptr) {
            throw std::runtime_error("bad value");
        }
        cur_pos += length;
        return ret;
    }

    float consumeFloat() {
        float ret;
        memcpy(&ret, buffer + cur_pos, sizeof(float));
        cur_pos += sizeof(float);
        return ret;
    }

    double consumeDouble() {
        double ret;
        memcpy(&ret, buffer + cur_pos, sizeof(double));
        cur_pos += sizeof(double);
        return ret;
    }

    uint8_t consumeByte() {
        return buffer[cur_pos++];
    }

    Buffer consumeBuffer(size_t size) {
        Buffer b;
        b.assign((char *)buffer + cur_pos, size);
        cur_pos += size;
        return b;
    }
};

std::shared_ptr<wasm::module_t> parse(std::string wasm_path);

class CodegenModifierBase {
    Buffer modifyModule(wasm::module_t * modu, Buffer in) {return in;}
};

class Codegen {
    Buffer encodeModule(wasm::base_node node);
    CodegenModifierBase modifier; 
};

}
