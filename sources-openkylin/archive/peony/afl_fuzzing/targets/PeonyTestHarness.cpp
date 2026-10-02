#include "PeonyTestHarness.h"
#include <file-info.h>
#include <file-info-job.h>
#include <file-enumerator.h>
#include <QCoreApplication>

bool PeonyTestHarness::initialized_ = false;
static QCoreApplication* g_app = nullptr;

void PeonyTestHarness::initialize() {
    if (!initialized_) {
        // 初始化Qt应用（仅一次）
        if (!qApp) {
            static int argc = 0;
            static char* argv[] = {nullptr};
            g_app = new QCoreApplication(argc, argv);
        }
        initialized_ = true;
    }
}

void PeonyTestHarness::cleanup() {
    // 清理资源
}

bool PeonyTestHarness::testFileInfoSync(const std::string& uri) {
    if (uri.empty()) return false;

    try {
        // 测试FileInfo::fromUri
        auto info1 = Peony::FileInfo::fromUri(uri.c_str());
        if (!info1) return false;

        // 验证基本属性
        volatile bool isEmpty = info1->isEmptyInfo();
        volatile std::string displayName = info1->displayName().toStdString();
        (void)isEmpty;
        (void)displayName;

        // 如果是file://开头，测试从path创建的一致性
        if (uri.substr(0, 7) == "file://") {
            std::string path = uri.substr(7);
            auto info2 = Peony::FileInfo::fromPath(path.c_str());
            if (info2) {
                volatile bool equal = (info1 == info2);
                (void)equal;
            }
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool PeonyTestHarness::testFileInfoJobSync(const std::string& uri) {
    if (uri.empty()) return false;

    try {
        // 使用同步API查询文件信息
        Peony::FileInfoJob job(uri.c_str());
        job.querySync();

        auto info = job.getInfo();
        if (info) {
            volatile bool isEmpty = info->isEmptyInfo();
            (void)isEmpty;
            return true;
        }

        return false;
    } catch (...) {
        return false;
    }
}

bool PeonyTestHarness::testFileEnumeratorSync(const std::string& uri) {
    if (uri.empty()) return false;

    try {
        // 使用同步API枚举目录
        Peony::FileEnumerator enumerator;
        enumerator.setEnumerateDirectory(uri.c_str());
        enumerator.setEnumerateWithInfoJob();
        enumerator.enumerateSync();

        auto uris = enumerator.getChildrenUris();
        volatile int count = uris.size();
        (void)count;

        auto infos = enumerator.getChildren();
        volatile int infoCount = infos.size();
        (void)infoCount;

        return true;
    } catch (...) {
        return false;
    }
}
