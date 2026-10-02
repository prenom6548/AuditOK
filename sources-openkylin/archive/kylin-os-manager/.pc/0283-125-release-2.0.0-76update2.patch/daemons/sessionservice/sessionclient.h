#ifndef SESSIONCLIENT_H
#define SESSIONCLIENT_H

#include <QObject>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusReply>
#include <QProcess>
#include <QString>
#include <QPointer>
#include "driverinstall/aptinstaller.h"
#include "service_support_popup.h"
#include "disk_monitor_exception_popup.h"

class SessionClient: public QObject {
    Q_OBJECT
public:
    SessionClient(QObject *parent = nullptr);

public Q_SLOTS:
    void onNewDeviceConnectSlot(QString devString);

private:
    void initHotplugClient();
    bool isjunkCleanBoxShow(); //用于避免同时弹多个清理窗
    QString m_driverInfo;
    QString m_devType;
    QString m_devModel;
    QString m_driverSize;
    AptInstaller *m_installer = nullptr;
    QString oninstall = "";
    QPointer<DiskMonitorExceptionPopup> log_size_exception_popup_;
    QPointer<DiskMonitorExceptionPopup> disk_space_full_popup_;
    QPointer<ServiceSupportPopup> service_support_popup_;

private Q_SLOTS:
    void actionInvokeSlot(quint32 id, QString actionkey);
    void showSuccessMsg();
    void showFailMsg();
    /**
     * @brief 启动服务支持全局反馈
     *
     * @param mode             模式(1: 用户主动触发 2: 崩溃后触发 3: 静默提交)
     * @param error_code       错误码(非必填，没有填空)
     * @param exec_name        二进制名
     * @param path             需要上报的附件路径
     * @param restart          十六进制编码的重启命令行
     * @param crash_stack_path 存储崩溃堆栈的文件路径
     *
     * @example dbus-send --system --type=signal /com/KylinOsManager/ServiceSupport com.KylinOsManager.ServiceSupport.RunGlobalFeedback int32:2 string:"" string:"kylin-os-manager" string:"/tmp/xxx/" string:"" string:""
     */
    void on_RunGlobalFeedback(int mode, QString error_code, QString exec_name, QString path, QString restart, QString crash_stack_path);
    void on_NotifyPopupLogFileSizeToplimit();
    void on_NotifyPopupDiskUsageRateToplimit();
    void on_NotifyPopupLogFileException();
    /**
     * @brief 内存使用超出阈值后，弹出气泡弹窗提示
     */
    void on_MemoryExceedsThreshold();
    void addWhiteToNetCtlFile(); /* 添加白名单 */
    void addWhiteToKysecNetctl(QString filePath); /* 添加白名单 */
};

#endif
