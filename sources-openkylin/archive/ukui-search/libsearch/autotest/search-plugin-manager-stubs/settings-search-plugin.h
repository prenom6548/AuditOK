#ifndef SETTINGS_SEARCH_PLUGIN_H
#define SETTINGS_SEARCH_PLUGIN_H

#include "search-plugin-test-stub.h"

namespace UkuiSearch {
class SettingsSearchPlugin : public SearchPluginTestStub
{public:
    explicit SettingsSearchPlugin(QObject *parent = nullptr)
        : SearchPluginTestStub(QStringLiteral("Settings Search"), parent)
    {
    }
};
}

#endif // SETTINGS_SEARCH_PLUGIN_H
