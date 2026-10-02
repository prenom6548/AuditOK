#ifndef DATA_COLLECTER_STUB_H
#define DATA_COLLECTER_STUB_H

#include <QString>

class DataCollecter
{
public:
    static void collectLaunchEvent(const QString &plugin, const QString &button)
    {
        lastPlugin = plugin;
        lastButton = button;
        ++calls;
    }

    static inline int calls = 0;
    static inline QString lastPlugin;
    static inline QString lastButton;
};

#endif
