#ifndef PEONY_TEST_HARNESS_H
#define PEONY_TEST_HARNESS_H

#include <string>
#include <memory>

namespace Peony {
    class FileInfo;
    class FileEnumerator;
}

/**
 * PeonyTestHarness - 隔离层
 * 仅暴露Peony的同步API，避免Qt事件循环依赖
 * 用于AFL模糊测试
 */
class PeonyTestHarness {
public:
    static void initialize();
    static void cleanup();

    // 同步API包装
    static bool testFileInfoSync(const std::string& uri);
    static bool testFileEnumeratorSync(const std::string& uri);
    static bool testFileInfoJobSync(const std::string& uri);

private:
    PeonyTestHarness() = default;
    static bool initialized_;
};

#endif // PEONY_TEST_HARNESS_H
