#ifndef KYLIN_OS_MANAGER_DAEMONS_SERVICE_TOOL_BOX_INTERFACE_H
#define KYLIN_OS_MANAGER_DAEMONS_SERVICE_TOOL_BOX_INTERFACE_H

#include <QDBusAbstractAdaptor>
#include <QProcess>
#include <QDBusContext>
#include <QByteArray>

#define TOOL_BOX_DBUS_NAME "com.kylin-os-manager"
#define TOOL_BOX_DBUS_PATH "/com/KylinOsManager/ToolBox"
#define TOOL_BOX_DBUS_INTERFACE "com.KylinOsManager.ToolBox"

namespace tool_box {

class ToolBoxAdaptor : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.KylinOsManager.ToolBox")
public:
    ToolBoxAdaptor(QObject *obj);

Q_SIGNALS:
    void UpdateSourceFinish(int exit_code);

public Q_SLOTS:
    void UpdateSource();
    void ChangePlace(int place);

private Q_SLOTS:
    void on_UpdateSourceFinish(int exit_code, QProcess::ExitStatus exit_status);

private:
    void HandleOutput(QByteArray &output);

    bool       ok_;
    QByteArray standard_output_buf_;
    QByteArray standard_error_buf_;
};

}

#endif
