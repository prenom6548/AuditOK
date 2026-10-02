#ifndef AI_SEARCH_PLUGIN_DIR_WATCHER_STUB_H
#define AI_SEARCH_PLUGIN_DIR_WATCHER_STUB_H

#include <QStringList>

namespace UkuiSearch {

extern QStringList g_blockDirsOfUser;

class DirWatcher
{
public:
    static DirWatcher *getDirWatcher()
    {
        static DirWatcher watcher;
        return &watcher;
    }

    QStringList getBlockDirsOfUser() const
    {
        return g_blockDirsOfUser;
    }
};

}  // namespace UkuiSearch

#endif
