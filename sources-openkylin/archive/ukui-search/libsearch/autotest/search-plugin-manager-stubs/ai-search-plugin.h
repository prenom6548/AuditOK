#ifndef AI_SEARCH_PLUGIN_H
#define AI_SEARCH_PLUGIN_H

#include "search-plugin-test-stub.h"

namespace UkuiSearch {
class AiSearchPlugin : public SearchPluginTestStub
{public:
    explicit AiSearchPlugin(QObject *parent = nullptr)
        : SearchPluginTestStub(QStringLiteral("Ai Search"), parent)
    {
    }
};
}

#endif // AI_SEARCH_PLUGIN_H
