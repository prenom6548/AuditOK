#ifndef APP_SEARCH_PLUGIN_H
#define APP_SEARCH_PLUGIN_H

#include "search-plugin-test-stub.h"

namespace UkuiSearch {
class AppSearchPlugin : public SearchPluginTestStub
{public:
    explicit AppSearchPlugin(QObject *parent = nullptr)
        : SearchPluginTestStub(QStringLiteral("Applications Search"), parent)
    {
    }
};
}

#endif // APP_SEARCH_PLUGIN_H
