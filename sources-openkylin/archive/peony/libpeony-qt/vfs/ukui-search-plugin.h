#ifndef UKUISEARCHPLUGIN_H
#define UKUISEARCHPLUGIN_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include "search-plugin-iface.h"
#include "ukui-search/ukui-search-task.h"
#include "ukui-search/data-queue.h"
#include "ukui-search/result-item.h"

namespace Peony {

enum PeonySearchStatus {
    SEARCHFILE,
    SEARCHFILECONTENT,
    SEARCHFILEING,
    SEARCHFILECONTENTING
};

class UkuiSearchPlugin : public QObject, public SearchPluginIface
{
    Q_OBJECT
public:
    explicit UkuiSearchPlugin(QObject *parent = nullptr);
    ~UkuiSearchPlugin();

    PluginType pluginType() override {
        return PluginType::SearchPlugin;
    }

    const QString name() override {
        return "Ukui-Search Plugin";
    }

    const QString description() override {
        return "Used ukui-search iface Plugin";
    }

    const QIcon icon() override {
        return QIcon();
    }

    void setEnable(bool enable) override {
        Q_UNUSED(enable);
    }

    bool isEnable() override {
        return true;
    }

    bool isSupportUri(const QString &uri) override;
    int priority() override;
    void initSearch(const QString &searchUri, const QStringList &searchDirs, const QStringList &searchKeys) override;
    void startSearch() override;
    void stopSearch() override;
    QQueue<QString>& getSearchResult() override;
    void resultDequeue() override;

    /** Thread-safe for GIO worker thread + main thread; copies m_resultQueue under lock. */
    void startSearchAndCopyResults(QQueue<QString> &out);

private:
    void startSearchUnlocked();
    QMutex m_mutex;
    UkuiSearch::UkuiSearchTask *m_search;
    UkuiSearch::DataQueue<UkuiSearch::ResultItem> *m_queue;
    UkuiSearch::UkuiSearchTask *m_contentSearch;
    UkuiSearch::DataQueue<UkuiSearch::ResultItem> *m_contentQueue;
    bool m_searchEngine;
    PeonySearchStatus m_searchStatus;
    PeonySearchStatus m_searchContent;
    QHash<QString, QString> *m_duplicatesHash;
    QQueue<QString> m_resultQueue;
};

}

#endif // UKUISEARCHPLUGIN_H
