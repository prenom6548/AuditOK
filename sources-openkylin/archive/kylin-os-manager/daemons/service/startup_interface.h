#include <QObject>
#include <QDBusContext>

namespace startup {

class Startup : public QObject, protected QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.KylinOsManager.Startup")
public:
    Startup(QObject *parent);

public Q_SLOTS:
    // 该接口用于启动服务
    // root 守护进程，改为默认不自启，提供 dbus 启动文件，当 session 服务启动时通过调用该接口，由 dbus-daemon 将服务拉起
    // 将启动时机由开机自启延迟到登录桌面后启动，解决影响开机时长等测试问题
    void Start();
};

}