#ifndef AI_SEARCH_PLUGIN_FILE_INDEXER_CONFIG_STUB_H
#define AI_SEARCH_PLUGIN_FILE_INDEXER_CONFIG_STUB_H

namespace UkuiSearch {

extern bool g_aiIndexEnabled;

class FileIndexerConfig
{
public:
    static FileIndexerConfig *getInstance()
    {
        static FileIndexerConfig config;
        return &config;
    }

    bool isAiIndexEnable() const
    {
        return g_aiIndexEnabled;
    }
};

}  // namespace UkuiSearch

#endif
