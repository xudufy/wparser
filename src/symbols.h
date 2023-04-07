#pragma once
#include "common.h"
#include <memory>

namespace wparser {

namespace wasm{

struct extent_t{
    size_t start = 0, end = 0;
};

class base_node {
public:
    base_node(const char * k) {
        kind = k;
    }
    const char *kind;
    extent_t extent;
    extent_t extent_without_size;
    inline std::string toString() {
        return "extent: " + std::to_string(extent.start)
        + " " + std::to_string(extent.end)
        + " " + std::to_string(extent_without_size.start)
        + " " + std::to_string(extent_without_size.end);
    }
};
class typesec_t: public base_node {
public:
    typesec_t():base_node("typesec"){}
};

struct limits_t{
    byte limitsType;
    u32 min_;
    u32 max_;
    byte sharedFlag = 0;
};

struct importdesc{
    struct base{
        std::string mod;
        std::string nm;
    };

    struct func: public base {
        u32 typeidx;
    };

    struct table: public base {
        s32 reftype;
        limits_t lim;
    };

    struct mem: public base {
        limits_t lim;
    };

    struct global: public base {
        s32 valtype;
        byte mut;
    };

};

class importsec_t: public base_node {
public:
    importsec_t():base_node("importsec"){}    
    std::vector<importdesc::func> funcs;
    std::vector<importdesc::table> tables;
    std::vector<importdesc::mem> mems;
    std::vector<importdesc::global> globals;
};
class funcsec_t: public base_node {
public:
    funcsec_t():base_node("funcsec"){}
};
class tablesec_t: public base_node {
public:
    tablesec_t():base_node("tablesec"){}
};
class memsec_t: public base_node {
public:
    memsec_t():base_node("memsec"){}
};
class globalsec_t: public base_node {
public:
    globalsec_t():base_node("globalsec"){}
};
class exportsec_t: public base_node {
public:
    exportsec_t():base_node("exportsec"){}
};
class startsec_t: public base_node {
public:
    startsec_t():base_node("startsec"){}
};
class elemsec_t: public base_node {
public:
    elemsec_t():base_node("elemsec"){}
};
class datacountsec_t: public base_node {
public:
    datacountsec_t():base_node("datacountsec"){}
};

class modulenamesubsec_t : public base_node {
public:
    modulenamesubsec_t():base_node("modulenamesubsec"){}
    std::string name;
};

class funcnamesubsec_t : public base_node {
public:
    funcnamesubsec_t():base_node("funcnamesubsec"){}
    std::unordered_map<u32, std::string> namemap;
};

class localnamesubsec_t : public base_node {
public:
    localnamesubsec_t():base_node("localnamesubsec"){}
    std::unordered_map<u32, std::unordered_map<u32, std::string>> indirectnamemap;
};

class namesec_t: public base_node {
public:
    namesec_t():base_node("namesec"){}
    std::shared_ptr<modulenamesubsec_t> modulenamesubsec = nullptr;
    std::shared_ptr<funcnamesubsec_t> funcnamesubsec = nullptr;
    std::shared_ptr<localnamesubsec_t> localnamesubsec = nullptr;
};

class func_t;

class expr_t : public base_node {
public:
    expr_t():base_node("expr"){}
    
    func_t* parentFunc;
};

enum class valtype_t: int8_t {i32 = -1, i64=-2, f32=-3, f64 = -4, v128=-5, funcref=-16, externref=-17,
functype=-32, resulttype=-64};

enum class WasmSectionEnum: uint8_t {
    Custom,
    Type,
    Import,
    Function,
    Table,
    Memory,
    Global,
    Export,
    Start,
    Element,
    Code,
    Data,
    DataCount,
};

class locals_t : public base_node {
public:
    locals_t():base_node("locals"){}

    u32 repeat;
    valtype_t valtype;
};


class func_t : public base_node {
public:
    func_t():base_node("func"){}

    std::shared_ptr<expr_t> expr;
    uint32_t funcidx;
    std::vector<std::shared_ptr<locals_t>> locals;
};

class codesec_t: public base_node {
public:
    codesec_t():base_node("codesec"){}

    std::vector<std::shared_ptr<func_t>> func;
};
class datasec_t: public base_node {
public:
    datasec_t():base_node("datasec"){}
};
class customsec_t: public base_node {
public:
    customsec_t():base_node("customsec"){}

    std::shared_ptr<namesec_t> namesec;
    std::string name;
};

class module_t: public base_node {
public:
    module_t():base_node("module"){}

    std::vector<std::shared_ptr<base_node>> children;
    std::vector<std::shared_ptr<customsec_t>> customsec;
    std::vector<std::shared_ptr<typesec_t>> typesec;
    std::vector<std::shared_ptr<importsec_t>> importsec;
    std::vector<std::shared_ptr<funcsec_t>> funcsec;
    std::vector<std::shared_ptr<tablesec_t>> tablesec;
    std::vector<std::shared_ptr<memsec_t>> memsec;
    std::vector<std::shared_ptr<globalsec_t>> globalsec;
    std::vector<std::shared_ptr<exportsec_t>> exportsec;
    std::vector<std::shared_ptr<startsec_t>> startsec;
    std::vector<std::shared_ptr<elemsec_t>> elemsec;
    std::vector<std::shared_ptr<codesec_t>> codesec;
    std::vector<std::shared_ptr<datasec_t>> datasec;
    std::vector<std::shared_ptr<datacountsec_t>> datacountsec;
};


}
}