#ifndef SEARCHPLUGINMANAGER_H
#define SEARCHPLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include "peony-core_global.h"
#include "search-plugin-iface.h"

namespace Peony {
class PEONYCORESHARED_EXPORT SearchPluginManager : public QObject
{
    Q_OBJECT
public:
    static SearchPluginManager *getInstance();

    void registerSearchPlugin(SearchPluginIface *plugin);

    void unregisterSearchPlugin(SearchPluginIface *plugin);

    SearchPluginIface *getDefaultSearchPlugin(const QString &uri);

    SearchPluginIface* getSearchPluginForName(const QString &pluginName);

private:
    explicit SearchPluginManager(QObject *parent = nullptr);
    ~SearchPluginManager();

    QMap<QString, SearchPluginIface *> m_pluginsMap;
    QMutex m_mutex;
};
}

#endif // SEARCHPLUGINMANAGER_H
