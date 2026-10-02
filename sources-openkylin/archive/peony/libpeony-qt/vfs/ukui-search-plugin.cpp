#include "ukui-search-plugin.h"
#include "file-utils.h"
#include <QMutexLocker>

using namespace Peony;

UkuiSearchPlugin::UkuiSearchPlugin(QObject *parent)
{
    m_search = new UkuiSearch::UkuiSearchTask();
    m_queue = m_search->init();
    m_contentSearch = new UkuiSearch::UkuiSearchTask();
    m_contentQueue = m_contentSearch->init();
    m_searchEngine = true;
    m_searchStatus = SEARCHFILE;
    m_searchContent = SEARCHFILECONTENT;
    m_duplicatesHash = new QHash<QString, QString>;
}

UkuiSearchPlugin::~UkuiSearchPlugin()
{
    delete m_duplicatesHash;
    m_duplicatesHash = nullptr;
    delete m_contentSearch;
    m_contentSearch = nullptr;
    delete m_search;
    m_search = nullptr;
}

bool UkuiSearchPlugin::isSupportUri(const QString &uri)
{
   if (uri.startsWith("file://")) {
       return true;
   }
   return false;
}

int UkuiSearchPlugin::priority()
{
    return 1;
}

void UkuiSearchPlugin::initSearch(const QString &searchUri, const QStringList &searchDirs, const QStringList &searchKeys)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(searchUri);
    if (nullptr != m_search && m_searchEngine) {
        m_search->clearAllConditions();
        m_queue->clear();

        for (const QString &dir : searchDirs) {
            QString encodeDir = Peony::FileUtils::urlDecode(dir);
            m_search->addSearchDir(encodeDir);
        }

        for (const QString &key : searchKeys) {
            m_search->addKeyword(key);
        }

        m_search->initSearchPlugin(UkuiSearch::SearchProperty::SearchType::File);
        m_queue = m_search->init();
        m_search->setMaxResultNum(9999999);

    }

    if (nullptr != m_contentSearch && m_searchEngine) {
        m_contentSearch->clearAllConditions();
        m_contentQueue->clear();

        for (const QString &dir : searchDirs) {
            QString encodeDir = Peony::FileUtils::urlDecode(dir);
            m_contentSearch->addSearchDir(encodeDir);
        }

        for (const QString &key : searchKeys) {
            m_contentSearch->addKeyword(key);
        }

        m_contentSearch->initSearchPlugin(UkuiSearch::SearchProperty::SearchType::FileContent);
        m_contentQueue = m_contentSearch->init();
        m_contentSearch->setMaxResultNum(9999999);
    }

    m_duplicatesHash->clear();
    m_resultQueue.clear();
    m_searchStatus = SEARCHFILE;
    m_searchContent = SEARCHFILECONTENT;
}

void UkuiSearchPlugin::startSearchAndCopyResults(QQueue<QString> &out)
{
    QMutexLocker locker(&m_mutex);
    startSearchUnlocked();
    out = m_resultQueue;
}

void UkuiSearchPlugin::startSearch()
{
    QMutexLocker locker(&m_mutex);
    startSearchUnlocked();
}

void UkuiSearchPlugin::startSearchUnlocked()
{
//    search_engine = search_enumerator->priv->search_engine;
    qDebug() << __func__ << m_searchStatus << m_searchContent;
    if (m_searchStatus == SEARCHFILE) {
        m_search->startSearch(UkuiSearch::SearchProperty::SearchType::File);
        m_searchStatus = SEARCHFILEING;
        qDebug() << __func__ << __LINE__ << "start file name search";
    }

    while (true) {
        if (!m_search->isSearching(UkuiSearch::SearchProperty::SearchType::File)
                || !m_queue->isEmpty()) {
            if (!m_search->isSearching(UkuiSearch::SearchProperty::SearchType::File)
                    && m_searchStatus == SEARCHFILEING) {
            }
            break;
        }
    }

    while (!m_queue->isEmpty() && m_searchEngine) {
        UkuiSearch::ResultItem resultItem = m_queue->dequeue();
        //qDebug() << "resultItem-->" << resultItem.getItemKey();
        QString path = resultItem.getItemKey();
        g_autofree gchar* encoded_path = g_uri_escape_string(path.toUtf8().constData(), ":/", false);
        path = encoded_path;
        QString uri = "file://" + path;
        m_duplicatesHash->insert(uri, "file");
        m_resultQueue.enqueue(uri);
    }

    if (m_searchContent == SEARCHFILECONTENT) {
        m_contentSearch->startSearch(UkuiSearch::SearchProperty::SearchType::FileContent);
        m_searchContent = SEARCHFILECONTENTING;
        qDebug() << __func__ << __LINE__ << "start file content search";
    }

    while (true) {
        if (!m_contentSearch->isSearching(UkuiSearch::SearchProperty::SearchType::FileContent)
                || !m_contentQueue->isEmpty()) {
            break;
        }
    }

    while (!m_contentQueue->isEmpty() && m_searchEngine) {
        UkuiSearch::ResultItem resultItem = m_contentQueue->dequeue();
        //qDebug() << "resultItem-->" << resultItem.getItemKey();
        QString path = resultItem.getItemKey();
        g_autofree gchar* encoded_path = g_uri_escape_string(path.toUtf8().constData(), ":/", false);
        path = encoded_path;
        QString uri = "file://" + path;
        if (m_duplicatesHash->contains(uri)) {
            continue;
        }
        m_duplicatesHash->insert(uri, "fileContent");
        m_resultQueue.enqueue(uri);
    }
}

void UkuiSearchPlugin::stopSearch()
{

}

QQueue<QString> &UkuiSearchPlugin::getSearchResult()
{
    return m_resultQueue;
}

void UkuiSearchPlugin::resultDequeue()
{
    QMutexLocker locker(&m_mutex);
    if (!m_resultQueue.isEmpty()) {
        m_resultQueue.dequeue();
    }
}
