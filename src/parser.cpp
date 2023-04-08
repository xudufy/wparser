#include "parser.h"
#include <iostream>

namespace wparser {

Buffer openFile(std::string path) {
    auto fp = fopen(path.c_str(), "rb");
    if (!fp) {
        throw std::runtime_error("bad file");
    }
    if (fseek(fp, 0, SEEK_END)) {
        throw std::runtime_error("bad file");
    }
    auto buffer_size = ftell(fp);
    auto buffer = (uint8_t*)malloc(buffer_size);
    rewind(fp);
    auto ret = fread(buffer, 1, buffer_size, fp);
    if ((u32)ret != (u32)buffer_size) {
        throw std::runtime_error("bad file");
    }
    fclose(fp);
    std::string buf;
    buf.assign((char *)buffer, buffer_size);
    return buf;
}

void writeFile(std::string path, Buffer buf) {
    auto fp = fopen(path.c_str(), "wb");
    if (!fp) {
        throw std::runtime_error("bad file");
    }
    auto ret = fwrite(buf.data(), 1, buf.size(), fp);
    if (ret != buf.size()) {
        throw std::runtime_error("bad file");
    }
}

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

std::string parseName(oBufferStream & source) {
    u32 nameLength = source.consumeUInt();
    return source.consumeBuffer(nameLength);
}

void parseVec(oBufferStream & source, std::function<void(oBufferStream &)> func_foreach) {
    u32 vecLength = source.consumeUInt();
    for (size_t i = 0; i < vecLength; i++) {
        func_foreach(source);
    }
}

std::shared_ptr<base_node> parseNameSec(oBufferStream & source, size_t extent_end) {
    auto namesec = std::make_shared<namesec_t>();
    //name already parsed in parseCustomSec
    while (source.cur_pos < extent_end) {
        u32 subsec_id = source.consumeByte();
        if (subsec_id == 0) {
            auto subsec = std::make_shared<modulenamesubsec_t>();
            parseLength(subsec.get(), -1, source, false);
            subsec->name = parseName(source);
            namesec->modulenamesubsec = subsec;
        } else if (subsec_id == 1) {
            auto subsec = std::make_shared<funcnamesubsec_t>();
            parseLength(subsec.get(), -1, source, false);
            parseVec(source, [&subsec](oBufferStream & source) {
                u32 idx = source.consumeUInt();
                std::string name = parseName(source);
                subsec->namemap[idx] = name;
            });
            namesec->funcnamesubsec = subsec;
        } else if (subsec_id == 2) {
            auto subsec = std::make_shared<localnamesubsec_t>();
            parseLength(subsec.get(), -1, source, false);
            parseVec(source, [&](oBufferStream & source) {
                u32 funcidx = source.consumeUInt();
                auto func_entry = subsec->indirectnamemap[funcidx] = {};
                parseVec(source, [&](oBufferStream & source){
                    u32 idx = source.consumeUInt();
                    std::string name = parseName(source);
                    func_entry[idx] = name;
                });
            });
            namesec->localnamesubsec = subsec;
        }
    }
    return namesec;
}

std::shared_ptr<base_node> parseCustomSec(oBufferStream & source) {
    auto customsec = std::make_shared<customsec_t>();
    parseLength(customsec.get(), -1, source, false);
    auto nameLength = source.consumeUInt();
    customsec->name = source.consumeBuffer(nameLength);
    if (customsec->name == "name") {
        auto namesec = parseNameSec(source, customsec->extent.end);
        namesec->extent = customsec->extent;
        namesec->extent_without_size = customsec->extent_without_size;
        customsec->namesec = std::static_pointer_cast<namesec_t>(namesec);
    }
    source.cur_pos = customsec->extent.end;
    return customsec;
}

limits_t parseLimits(oBufferStream & source) {
    limits_t l;
    l.limitsType = source.consumeByte();
    l.min_ = source.consumeUInt();
    if (l.limitsType == 0x01 || l.limitsType == 0x03) {
        l.max_ = source.consumeUInt();
    }
    if (l.limitsType == 0x02 || l.limitsType == 0x03) {
        l.sharedFlag = 1;
    }
    return l;
}

std::shared_ptr<base_node> parseTypeSec(oBufferStream & source) {
    auto typesec = std::make_shared<typesec_t>();
    parseLength(typesec.get(), -1, source, true);
    return typesec;
} 


std::shared_ptr<base_node> parseImportSec(oBufferStream & source) {
    auto importsec = std::make_shared<importsec_t>();
    parseLength(importsec.get(), -1, source, false);
    parseVec(source, [&](oBufferStream & source) {
        auto mod = parseName(source);
        auto nm = parseName(source);
        auto importdesctype = source.consumeByte();
        switch (importdesctype) {
            case 0x00: {
                importdesc::func f;
                f.nm = std::move(nm);
                f.mod = std::move(mod);
                f.typeidx = source.consumeUInt();
                importsec->funcs.emplace_back(std::move(f));
                break;
            }
            case 0x01: {
                importdesc::table t;
                t.nm = std::move(nm);
                t.mod = std::move(mod);
                t.reftype = source.consumeSInt();
                t.lim = parseLimits(source);
                importsec->tables.emplace_back(std::move(t));
                break;
            }
            case 0x02: {
                importdesc::mem m;
                m.nm = std::move(nm);
                m.mod = std::move(mod);
                m.lim = parseLimits(source);
                importsec->mems.emplace_back(std::move(m));
                break;
            }
            case 0x03: {
                importdesc::global g;
                g.nm = std::move(nm);
                g.mod = std::move(mod);
                g.valtype = source.consumeSInt();
                g.mut = source.consumeByte();
                importsec->globals.emplace_back(std::move(g));
                break;
            }
            default: 
                throw std::runtime_error("bad importdesctype" + std::to_string(importdesctype));
        };
    });
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
        expr->extent.start = source.cur_pos;
        expr->extent.end = func->extent.end;
        expr->extent_without_size = expr->extent;
        expr->parentFunc = func.get();
        //TODO: parse expr;
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

void postProcessIdx(module_t *module_) {
    u32 importFuncCount = 0;
    for (auto & importsec: module_->importsec) {
        importFuncCount += importsec->funcs.size();
    }
    for (auto & codesec: module_->codesec) {
        for (auto & func: codesec->func) {
            func->funcidx = importFuncCount++;
        }
    }
}

void postProcessFuncName(module_t *module_) {
    std::unordered_map<u32, std::string> funcNameMap;
    for (auto & custom : module_->customsec) {
        if (custom->namesec != nullptr) {
            auto namesec = custom->namesec;
            if (namesec->funcnamesubsec) {
                funcNameMap.merge(namesec->funcnamesubsec->namemap);
            }
        }
    }
    for (auto & codesec: module_->codesec) {
        for (auto & func: codesec->func) {
            if (funcNameMap.count(func->funcidx) == 1) {
                func->debuginfo_name = funcNameMap[func->funcidx];
            }
        }
    }
}

std::shared_ptr<module_t> parseModule(oBufferStream & source) {
    auto modu = std::make_shared<module_t>();
    modu->extent.start = 0;
    modu->extent.end = source.buffer_size;
    modu->extent_without_size = modu->extent;
    while (!source.eof()) {
        auto sectionId = (WasmSectionEnum)source.consumeByte();
#define PARSE_CHILDREN_SNIPPET(CAMELCASE, LOWERCASE) \
            if (sectionId == WasmSectionEnum::CAMELCASE) { \
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
    postProcessFuncName(modu.get());
    return modu;
}


static const uint8_t wasmOpening[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
std::shared_ptr<module_t> parse(std::string wasm_path){
    oBufferStream source;
    source.openFile(wasm_path);
    
    std::string opening = source.consumeBuffer(8);
    if (memcmp(opening.data(), wasmOpening, 8) != 0){
        goto err;
    }

    return parseModule(source);
err:
    source.close();
    throw std::runtime_error("bad file");
}


};
