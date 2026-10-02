#ifndef WEB_SEARCH_PLUGIN_H
#define WEB_SEARCH_PLUGIN_H

#include "search-plugin-test-stub.h"

namespace UkuiSearch {
class WebSearchPlugin : public SearchPluginTestStub
{public:
    explicit WebSearchPlugin(QObject *parent = nullptr)
        : SearchPluginTestStub(QStringLiteral("Web Page"), parent)
    {
    }
};
}

#endif // WEB_SEARCH_PLUGIN_H
