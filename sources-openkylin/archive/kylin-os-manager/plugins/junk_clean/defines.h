#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_DEFINES_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_DEFINES_H

#include <QString>
#include <QMap>
#include <QList>
#include <QMetaType>

#define JUNK_CLEAN_DBUS_NAME "com.kylin-os-manager"
#define JUNK_CLEAN_DBUS_PATH "/com/KylinOsManager/JunkClean"
#define JUNK_CLEAN_DBUS_INTERFACE "com.KylinOsManager.JunkClean"

namespace junk_clean {

enum class Type {
    kSystemJunk,
    kInternetJunk,
    kUsageTraces
};

enum class Level {
    kNoEffect,    // 无影响
    kGeneral,     // 一般级别，默认不选中
    kCareful,     // 谨慎级别，默认不选中 + 文案提醒
};

struct CleanUpItem {
    QString mark;
    QString name;
    QString describe;
    enum Level level;
};
typedef QMap<junk_clean::Type, QList<CleanUpItem>> TypeCleanUpItems;

struct JunkItem {
    quint64 mark;
    QString path;
    quint64 size;
};

struct CleanTask {
    QString cleaner_mark;
    QList<quint64> junk_marks;
};

}

Q_DECLARE_METATYPE(junk_clean::TypeCleanUpItems)
Q_DECLARE_METATYPE(junk_clean::JunkItem)
Q_DECLARE_METATYPE(junk_clean::CleanTask)

#endif
