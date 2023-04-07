#pragma once
#include "common.h"
#include "symbols.h"

namespace wparser{


class CodegenModifierBase {
public:
    Buffer modifyModule(wasm::module_t * node, Buffer in) {return in;}
    Buffer modifyCustomSec(wasm::customsec_t* node, Buffer in) {return in;}
    Buffer modifyTypeSec(wasm::typesec_t* node, Buffer in) {return in;}
    Buffer modifyImportSec(wasm::importsec_t* node, Buffer in) {return in;}
    Buffer modifyFunctionSec(wasm::funcsec_t* node, Buffer in) {return in;}
    Buffer modifyTableSec(wasm::tablesec_t* node, Buffer in) {return in;}
    Buffer modifyMemorySec(wasm::memsec_t* node, Buffer in) {return in;}
    Buffer modifyGlobalSec(wasm::globalsec_t* node, Buffer in) {return in;}
    Buffer modifyExportSec(wasm::exportsec_t* node, Buffer in) {return in;}
    Buffer modifyStartSec(wasm::startsec_t* node, Buffer in) {return in;}
    Buffer modifyElementSec(wasm::elemsec_t* node, Buffer in) {return in;}
    Buffer modifyCodeSec(wasm::codesec_t* node, Buffer in) {return in;}
    Buffer modifyDataSec(wasm::datasec_t* node, Buffer in) {return in;}
    Buffer modifyDataCountSec(wasm::datacountsec_t* node, Buffer in) {return in;}
    Buffer modifyFunc(wasm::func_t* node, Buffer in) {return in;}
    Buffer modifyExpr(wasm::expr_t* node, Buffer in) {return in;}
};

class Codegen {
public:
    Buffer encodeModule(wasm::module_t* node);
private:
    Buffer encodeCustomSec(wasm::customsec_t* node);
    Buffer encodeTypeSec(wasm::typesec_t* node);
    Buffer encodeImportSec(wasm::importsec_t* node);
    Buffer encodeFunctionSec(wasm::funcsec_t* node);
    Buffer encodeTableSec(wasm::tablesec_t* node);
    Buffer encodeMemorySec(wasm::memsec_t* node);
    Buffer encodeGlobalSec(wasm::globalsec_t* node);
    Buffer encodeExportSec(wasm::exportsec_t* node);
    Buffer encodeStartSec(wasm::startsec_t* node);
    Buffer encodeElementSec(wasm::elemsec_t* node);
    Buffer encodeCodeSec(wasm::codesec_t* node);
    Buffer encodeDataSec(wasm::datasec_t* node);
    Buffer encodeDataCountSec(wasm::datacountsec_t* node);
    Buffer encodeFunc(wasm::func_t* node);
    Buffer encodeLimits(wasm::limits_t node);
    Buffer encodeExpr(wasm::expr_t* node);
    Buffer encodeValType(wasm::valtype_t node);
    Buffer encodeLocals(wasm::locals_t* node);

    Buffer refBufferByExtent(wasm::extent_t extent);
public:
    inline Codegen(std::shared_ptr<CodegenModifierBase> modifier, Buffer extent_ref_buffer): modifier(modifier), extent_ref_buffer(extent_ref_buffer) {
        if (modifier == nullptr) {
            modifier = std::make_shared<CodegenModifierBase>();
        }
    } 
    ~Codegen() = default;
    
    inline static std::shared_ptr<Codegen> create(std::shared_ptr<CodegenModifierBase> modifier = nullptr, Buffer extent_ref_buffer = "") {
        return std::make_shared<Codegen>(modifier, extent_ref_buffer);
    }

    std::shared_ptr<CodegenModifierBase> modifier;
    Buffer extent_ref_buffer;
};

}