#include "../src/wparser.h"
#include "../src/codegen.h"

int main(int argc, char*argv[]) {
    if (argc < 2) {
        return -1;
    }

    auto module_ = wparser::parse(argv[1]);
    W_LOG("module size: %zu", module_->extent.end);
    auto raw_content = wparser::openFile(argv[1]);
    auto codegen = std::make_shared<wparser::Codegen>(nullptr, raw_content);
    auto gen_content = codegen->encodeModule(module_.get());
    wparser::writeFile(std::string(argv[1]) + ".gen.wasm", gen_content);
    return 0;
}