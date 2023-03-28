#include "../src/wparser.h"

int main(int argc, char*argv[]) {
    if (argc < 2) {
        return -1;
    }

    auto mod = wparser::parse(argv[1]);
    W_LOG("%zu\n", mod->extent.end);
    for (size_t i = 0; i < mod->children.size(); i++) {
        W_LOG("%s", mod->children[i]->kind);
    }

    return 0;
}