#include "wparser.h"
#include <iostream>

namespace wparser {


void oBufferStream::openFile(std::string path) {
    auto fp = fopen(path.c_str(), "rb");
    if (!fp) {
        throw std::runtime_error("bad file");
    }
    if (fseek(fp, 0, SEEK_END)) {
        throw std::runtime_error("bad file");
    }
    this->buffer_size = ftell(fp);
    this->buffer = (uint8_t*)malloc(this->buffer_size);
    rewind(fp);
    auto ret = fread(this->buffer, 1, this->buffer_size, fp);
    if (ret != this->buffer_size) {
        throw std::runtime_error("bad file");
    }
    fclose(fp);
}

void oBufferStream::openBuffer(Buffer b) {
    buffer = (uint8_t*)malloc(b.size());
    buffer_size = b.size();
    memcpy(buffer, b.data(), buffer_size);
    cur_pos = 0;    
}

using namespace wasm;
using Byte = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s32 = int32_t;
using s64 = int64_t;

void parseLength(base_node* base_node, int start_offset, oBufferStream & source, bool skip = false) {
    auto start_pos = source.cur_pos;
    auto length = source.consumeUInt();
    base_node->extent.start = start_pos + start_offset;
    base_node->extent.end = source.cur_pos + length;
    base_node->extent_without_size.start = source.cur_pos;
    base_node->extent_without_size.end = base_node-> extent.end;
    if (skip) {
        source.cur_pos = base_node->extent.end;
    }
}

std::shared_ptr<base_node> parseNameSec(oBufferStream & source) {
    source.consumeByte(); // section id 0
    auto namesec = std::make_shared<namesec_t>();
    parseLength(namesec.get(), -1, source, false);
    
}

std::shared_ptr<base_node> parseCustomSec(oBufferStream & source) {
    auto customsec = std::make_shared<customsec_t>();
    parseLength(customsec.get(), -1, source, false);
    auto nameLength = source.consumeUInt();
    for (u64 i = 0; i < nameLength; i++) {
        customsec->name += source.consumeByte();
    }
    if (customsec->name == "name") {
        auto namesec = parseNameSec(source);
        customsec->namesec = std::static_pointer_cast<namesec_t>(namesec);
    }
    source.cur_pos = customsec->extent.end;
    return customsec;
}

std::shared_ptr<base_node> parseTypeSec(oBufferStream & source) {
    auto typesec = std::make_shared<typesec_t>();
    parseLength(typesec.get(), -1, source, true);
    return typesec;
}

std::shared_ptr<base_node> parseImportSec(oBufferStream & source) {
    auto importsec = std::make_shared<importsec_t>();
    parseLength(importsec.get(), -1, source, true);
    return importsec;
}

std::shared_ptr<base_node> parseFunctionSec(oBufferStream & source) {
    auto funcsec = std::make_shared<funcsec_t>();
    parseLength(funcsec.get(), -1, source, true);
    return funcsec;
}

std::shared_ptr<base_node> parseTableSec(oBufferStream & source) {
    auto tablesec = std::make_shared<tablesec_t>();
    parseLength(tablesec.get(), -1, source, true);
    return tablesec;
}

std::shared_ptr<base_node> parseMemorySec(oBufferStream & source) {
    auto memsec = std::make_shared<memsec_t>();
    parseLength(memsec.get(), -1, source, true);
    return memsec;
}

std::shared_ptr<base_node> parseGlobalSec(oBufferStream & source) {
    auto globalsec = std::make_shared<globalsec_t>();
    parseLength(globalsec.get(), -1, source, true);
    return globalsec;
}

std::shared_ptr<base_node> parseExportSec(oBufferStream & source) {
    auto exportsec = std::make_shared<exportsec_t>();
    parseLength(exportsec.get(), -1, source, true);
    return exportsec;
}

std::shared_ptr<base_node> parseStartSec(oBufferStream & source) {
    auto startsec = std::make_shared<startsec_t>();
    parseLength(startsec.get(), -1, source, true);
    return startsec;
}

std::shared_ptr<base_node> parseElementSec(oBufferStream & source) {
    auto elemsec = std::make_shared<elemsec_t>();
    parseLength(elemsec.get(), -1, source, true);
    return elemsec;
}

std::shared_ptr<base_node> parseCodeSec(oBufferStream & source) {
    auto codesec = std::make_shared<codesec_t>();
    parseLength(codesec.get(), -1, source);
    uint32_t func_size = source.consumeUInt();
    for (size_t i = 0; i < func_size; i++) {
        std::shared_ptr<func_t> func = std::make_shared<func_t>();
        parseLength(func.get(), 0, source);
        uint32_t local_size = source.consumeUInt();
        for (size_t j=0; j < local_size; j++) {
            auto local = std::make_shared<locals_t>();
            local->extent.start = source.cur_pos;
            local->repeat = source.consumeUInt();
            local->valtype = (valtype_t)source.consumeSInt();
            local->extent.end = source.cur_pos;
            local->extent_without_size = local->extent;
            func->locals.push_back(local);
        }
        auto expr = std::make_shared<expr_t>();
        //TODO: parse expr;
        expr->extent.start = source.cur_pos;
        expr->extent.end = func->extent.end;
        func->expr = expr;
        source.cur_pos = func->extent.end;
        codesec->func.push_back(func);
    }
    return codesec;
}

std::shared_ptr<base_node> parseDataSec(oBufferStream & source) {
    auto datasec = std::make_shared<datasec_t>();
    parseLength(datasec.get(), -1, source, true);
    return datasec;
}

std::shared_ptr<base_node> parseDataCountSec(oBufferStream & source) {
    auto datacountsec = std::make_shared<datacountsec_t>();
    parseLength(datacountsec.get(), -1, source, true);
    return datacountsec;
}

void postProcessIdx(module_t *module) {
    //TODO: populate funcidx.
} 

std::shared_ptr<module_t> parseModule(oBufferStream & source) {
    auto modu = std::make_shared<module_t>();
    modu->extent.start = 0;
    modu->extent.end = source.buffer_size;
    while (!source.eof()) {
        auto sectionId = (WasmSection)source.consumeByte();
#define PARSE_CHILDREN_SNIPPET(CAMELCASE, LOWERCASE) \
            if (sectionId == WasmSection::CAMELCASE) { \
            auto child = parse##CAMELCASE##Sec(source); \
            modu->children.push_back(child); \
            modu->LOWERCASE##sec.push_back(std::static_pointer_cast<LOWERCASE##sec_t>(child));} 
        
        PARSE_CHILDREN_SNIPPET(Custom, custom)
        PARSE_CHILDREN_SNIPPET(Type, type)
        PARSE_CHILDREN_SNIPPET(Import, import)
        PARSE_CHILDREN_SNIPPET(Function, func)
        PARSE_CHILDREN_SNIPPET(Table, table)
        PARSE_CHILDREN_SNIPPET(Memory, mem)
        PARSE_CHILDREN_SNIPPET(Global, global)
        PARSE_CHILDREN_SNIPPET(Export, export)
        PARSE_CHILDREN_SNIPPET(Start, start)
        PARSE_CHILDREN_SNIPPET(Element, elem)
        PARSE_CHILDREN_SNIPPET(Code, code)
        PARSE_CHILDREN_SNIPPET(Data, data)
        PARSE_CHILDREN_SNIPPET(DataCount, datacount)
#undef PARSE_CHILDREN_SNIPPET
        if ((int)sectionId > 12) {
            fprintf(stderr, "bad section id %d", (int)sectionId);
            throw std::runtime_error("bad section id");
        }
    }
    postProcessIdx(modu.get());
    
    return modu;
}


static const uint8_t wasmOpening[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
std::shared_ptr<module_t> parse(std::string wasm_path){
    oBufferStream source;
    source.openFile(wasm_path);
    
    uint8_t opening[8];
    source.consumeBuffer(opening, 8);
    if (memcmp(opening, wasmOpening, 8) != 0){
        goto err;
    }

    return parseModule(source);
err:
    source.close();
    throw std::runtime_error("bad file");
}


};
