#ifndef KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_APP_LAUNCHER_DBUS_APP_EXIT_H
#define KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_APP_LAUNCHER_DBUS_APP_EXIT_H

#include <QObject>
#include <QTimer>
#include <QApplication>

static const QString DBUS_NAME = "com.kylin.treasurebox";
static const QString DBUS_PATH = "/com/kylin/treasurebox";
static const QString DBUS_INTERFACE = "com.kylin.treasurebox";

class ExitGuard: public QObject {
    Q_OBJECT
public:
    ~ExitGuard() {
        QTimer::singleShot(0, qApp, &QApplication::quit);
    }
};

class DBusAppExit: public QObject {
    Q_OBJECT
public:
    DBusAppExit(const QString &uuid);

public Q_SLOTS:
    void on_Exit(QString uuid, QString data);

private:
    enum class Category {
        kNone,
        kFeature,            // 特色工具类
        kDebug,              // 运维调试类
        kTroubleshooting,    // 故障修复类
        kOther,              // 其他分类
    };
    struct AppInformation {
        AppInformation()
            : app_type(Category::kNone)
        {

        }

        QString       name;
        enum Category app_type;
    };

    void GetAppInformation();

    QString        uuid_;
    AppInformation app_information_;
};

#endif
