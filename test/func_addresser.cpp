#include "../src/wparser.h"

int main(int argc, char*argv[]) {
    if (argc < 2) {
        return -1;
    }

    auto mod = wparser::parse(argv[1]);
    W_LOG("module size: %zu", mod->extent.end);
    for (size_t i = 0; i < mod->children.size(); i++) {
        W_LOG("%s 0x%zx", mod->children[i]->kind, mod->children[i]->extent.end);
    }

    return 0;
}