#include "codegen.h"
#include "utils/LEB128.h"

namespace wparser {
    using namespace wasm;
    static Buffer cstr2buffer(uint8_t *c_str, size_t size) {
        std::string out;
        out.assign((char *)c_str, size);
        return out;
    }

    static Buffer encodeULEB128(u64 num, unsigned padto = 0) {
        char buf[32];
        auto length = encodeULEB128(num, (uint8_t *)buf, padto);
        return cstr2buffer((uint8_t *)buf, length);
    }

    static Buffer encodeSLEB128(s64 num, unsigned padto = 0) {
        char buf[32];
        auto length = encodeSLEB128(num, (uint8_t *)buf, padto);
        return cstr2buffer((uint8_t *)buf, length);
    }

    static Buffer encodeByte(byte b) {
        return cstr2buffer((uint8_t *)&b, 1);
    }

    static Buffer encodeVec(const std::vector<Buffer> & vec) {
        Buffer out = encodeULEB128(vec.size());
        for (auto & item: vec) {
            out += item;
        }
        return out;
    }

    static const uint8_t wasmOpening[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
    Buffer Codegen::encodeModule(wasm::module_t* node) {
        Buffer out;
        out.assign((char *)wasmOpening, 8);
#define GEN_SECTION_SNIPPET(Camelcase, Lowercase) \
        for (auto &subnode: node->Lowercase##sec) { \
            Buffer subbuffer = encode##Camelcase##Sec(subnode.get()); \
            out += encodeByte((byte)WasmSectionEnum::Camelcase) + \
                    encodeULEB128(subbuffer.size()) + subbuffer;\
        }

        GEN_SECTION_SNIPPET(Custom, custom)
        GEN_SECTION_SNIPPET(Type, type)
        GEN_SECTION_SNIPPET(Import, import)
        GEN_SECTION_SNIPPET(Function, func)
        GEN_SECTION_SNIPPET(Table, table)
        GEN_SECTION_SNIPPET(Memory, mem)
        GEN_SECTION_SNIPPET(Global, global)
        GEN_SECTION_SNIPPET(Export, export)
        GEN_SECTION_SNIPPET(Start, start)
        GEN_SECTION_SNIPPET(Element, elem)
        
        //In practice, datacount section must be before the code section to make the validator happy.
        GEN_SECTION_SNIPPET(DataCount, datacount)
        GEN_SECTION_SNIPPET(Code, code)
        GEN_SECTION_SNIPPET(Data, data)
#undef GEN_SECTION_SNIPPET
        return out;
    }

    Buffer Codegen::refBufferByExtent(extent_t extent) {
        return this->extent_ref_buffer.substr(extent.start, extent.end - extent.start);
    }

    Buffer Codegen::encodeCustomSec(wasm::customsec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyCustomSec(node, out);
    }
    Buffer Codegen::encodeTypeSec(wasm::typesec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyTypeSec(node, out);
    }
    Buffer Codegen::encodeImportSec(wasm::importsec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyImportSec(node, out);
    }
    Buffer Codegen::encodeFunctionSec(wasm::funcsec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyFunctionSec(node, out);
    }
    Buffer Codegen::encodeTableSec(wasm::tablesec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyTableSec(node, out);
    }
    Buffer Codegen::encodeMemorySec(wasm::memsec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyMemorySec(node, out);
    }
    Buffer Codegen::encodeGlobalSec(wasm::globalsec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyGlobalSec(node, out);
    }
    Buffer Codegen::encodeExportSec(wasm::exportsec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyExportSec(node, out);
    }
    Buffer Codegen::encodeStartSec(wasm::startsec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyStartSec(node, out);
    }
    Buffer Codegen::encodeElementSec(wasm::elemsec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyElementSec(node, out);
    }
    Buffer Codegen::encodeCodeSec(wasm::codesec_t* node) {
        Buffer out;
        std::vector<Buffer> funcs_out;
        for (auto & item: node->func) {
            funcs_out.emplace_back(std::move(encodeFunc(item.get())));
        }
        out = encodeVec(funcs_out);
        return modifier->modifyCodeSec(node, out);
    }
    Buffer Codegen::encodeDataSec(wasm::datasec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyDataSec(node, out);
    }
    Buffer Codegen::encodeDataCountSec(wasm::datacountsec_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyDataCountSec(node, out);
    }
    Buffer Codegen::encodeFunc(wasm::func_t* node) {
        Buffer out;
        std::vector<Buffer> locals;
        for (auto & item:node->locals) {
            locals.emplace_back(std::move(encodeLocals(item.get())));
        };
        out += encodeVec(locals);
        out += encodeExpr(node->expr.get());
        out = modifier->modifyFunc(node, out);
        auto length = out.size();
        out = encodeULEB128(length) + out;
        return out;
    }
    Buffer Codegen::encodeLimits(wasm::limits_t node) {
        return encodeByte(node.limitsType) + encodeULEB128(node.min_) + 
            ((node.limitsType & 0x01)? encodeULEB128(node.max_) : "");
    }
    Buffer Codegen::encodeExpr(wasm::expr_t* node) {
        auto out = refBufferByExtent(node->extent_without_size);
        return modifier->modifyExpr(node, out);
    }
    Buffer Codegen::encodeValType(wasm::valtype_t node) {
        return encodeSLEB128((int8_t)node);
    }
    Buffer Codegen::encodeLocals(wasm::locals_t* node) {
        return encodeULEB128(node->repeat) + encodeValType(node->valtype);
    }
}