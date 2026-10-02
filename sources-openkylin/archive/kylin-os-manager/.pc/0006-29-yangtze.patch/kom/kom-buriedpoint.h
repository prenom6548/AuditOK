#ifndef __KOM_BURIEDPOINT_H__
#define __KOM_BURIEDPOINT_H__
#include <QString>
#include <QMap>
namespace kom
{

enum BuriedPointType {
    MainApplication = 0,
    PluginsServiceSupport,
    PluginsToolBox,
    PluginsWinDataMigration,
    PluginsRemoteAssistanceTool,

    ServiceMonitor
};
class BuriedPoint
{
public:
    BuriedPoint() {}
    ~BuriedPoint() {}
    static bool uploadMessage(BuriedPointType messageType, QMap<QString, QString> data);
    static bool uploadMessage(BuriedPointType messageType, const QString &key, const QString &value);
    static bool uploadMessage(BuriedPointType messageType, const QString &msg);

private:
    static QString getBuriedPointTypeString(BuriedPointType type);
};
} // namespace kom

#endif
