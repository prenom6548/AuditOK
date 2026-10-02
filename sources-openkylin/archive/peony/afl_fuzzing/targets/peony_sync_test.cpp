#include <iostream>
#include <fstream>
#include <string>
#include "PeonyTestHarness.h"

// AFL持久化模式支持
#ifdef __AFL_HAVE_MANUAL_CONTROL
  #define AFL_LOOP(x) __AFL_LOOP(x)
#else
  #define AFL_LOOP(x) ((void)(x), 1)
#endif

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    // 一次性初始化
    PeonyTestHarness::initialize();

    // AFL持久化模式循环
    while (AFL_LOOP(1000)) {
        std::ifstream file(argv[1]);
        if (!file.is_open()) {
            continue;
        }

        std::string input((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
        file.close();

        if (input.empty()) continue;

        // 测试同步API
        PeonyTestHarness::testFileInfoSync(input);
        PeonyTestHarness::testFileInfoJobSync(input);
        PeonyTestHarness::testFileEnumeratorSync(input);
    }

    PeonyTestHarness::cleanup();
    return 0;
}
