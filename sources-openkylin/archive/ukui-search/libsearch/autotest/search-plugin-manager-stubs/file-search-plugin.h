#ifndef FILE_SEARCH_PLUGIN_H
#define FILE_SEARCH_PLUGIN_H

#include "search-plugin-test-stub.h"

namespace UkuiSearch {
class DirSearchPlugin : public SearchPluginTestStub
{public:
    explicit DirSearchPlugin(QObject *parent = nullptr)
        : SearchPluginTestStub(QStringLiteral("Dir Search"), parent)
    {
    }
};

class FileSearchPlugin : public SearchPluginTestStub
{public:
    explicit FileSearchPlugin(QObject *parent = nullptr)
        : SearchPluginTestStub(QStringLiteral("File Search"), parent)
    {
    }
};

class FileContentSearchPlugin : public SearchPluginTestStub
{public:
    explicit FileContentSearchPlugin(QObject *parent = nullptr)
        : SearchPluginTestStub(QStringLiteral("File Content Search"), parent)
    {
    }
};
}

#endif // FILE_SEARCH_PLUGIN_H
