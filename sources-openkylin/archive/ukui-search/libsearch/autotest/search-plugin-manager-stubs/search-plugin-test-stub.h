#ifndef SEARCH_PLUGIN_TEST_STUB_H
#define SEARCH_PLUGIN_TEST_STUB_H

#include <QDir>
#include <QPluginLoader>

#include "plugininterface/search-plugin-iface.h"

namespace UkuiSearch {

class SearchPluginTestStub : public QObject, public SearchPluginIface
{
public:
    explicit SearchPluginTestStub(const QString &pluginName, QObject *parent = nullptr)
        : QObject(parent),
          m_name(pluginName)
    {
    }

    PluginType pluginType() override { return PluginType::SearchPlugin; }
    const QString name() override { return m_name; }
    const QString description() override { return m_name; }
    const QIcon icon() override { return QIcon(); }
    void setEnable(bool enable) override { m_enable = enable; }
    bool isEnable() override { return m_enable; }
    QString getPluginName() override { return m_name; }
    size_t KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult) override
    {
        Q_UNUSED(keyword)
        Q_UNUSED(searchResult)
        return 0;
    }
    void stopSearch() override { ++m_stopCount; }
    QList<Actioninfo> getActioninfo(int type) override
    {
        Q_UNUSED(type)
        return {};
    }
    void openAction(int actionkey, QString key, int type) override
    {
        Q_UNUSED(actionkey)
        Q_UNUSED(key)
        Q_UNUSED(type)
    }
    QWidget *detailPage(const ResultInfo &ri) override
    {
        Q_UNUSED(ri)
        return nullptr;
    }
    int stopCount() const { return m_stopCount; }

private:
    QString m_name;
    bool m_enable = true;
    int m_stopCount = 0;
};

} // namespace UkuiSearch

#endif // SEARCH_PLUGIN_TEST_STUB_H
