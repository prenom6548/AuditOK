#ifndef NOTE_SEARCH_PLUGIN_H
#define NOTE_SEARCH_PLUGIN_H

#include "search-plugin-test-stub.h"

namespace UkuiSearch {
class NoteSearchPlugin : public SearchPluginTestStub
{public:
    explicit NoteSearchPlugin(QObject *parent = nullptr)
        : SearchPluginTestStub(QStringLiteral("Note Search"), parent)
    {
    }
};
}

#endif // NOTE_SEARCH_PLUGIN_H
