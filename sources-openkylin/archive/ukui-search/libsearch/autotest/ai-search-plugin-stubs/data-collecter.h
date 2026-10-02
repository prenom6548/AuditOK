#ifndef AI_SEARCH_PLUGIN_DATA_COLLECTER_STUB_H
#define AI_SEARCH_PLUGIN_DATA_COLLECTER_STUB_H

#include <QString>

class DataCollecter
{
public:
    static int calls;
    static QString lastPlugin;
    static QString lastButton;

    static void collectLaunchEvent(const QString &plugin, const QString &button)
    {
        ++calls;
        lastPlugin = plugin;
        lastButton = button;
    }
};

#endif
