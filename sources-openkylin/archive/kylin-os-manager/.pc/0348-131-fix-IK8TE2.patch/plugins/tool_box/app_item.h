#ifndef KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_APP_ITEM_H
#define KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_APP_ITEM_H

#include <future>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QEvent>
#include <QVariant>
#include <ktoolbutton.h>
#include "text_label.h"
#include "defines.h"

namespace tool_box {

class AppItem: public QWidget {
    Q_OBJECT
public:
    enum class Type {
        kExec,
        kDBus,
    };
    enum class Category {
        kNone,
        kFeature,            // 特色工具
        kDebug,              // 运维调试类
        kTroubleshooting,    // 故障修复类
        kOther,              // 其他分类
    };
    enum IconType {
        kMain = 0,
        kBackup
    };
    enum class DependPackageState {
        kUnknow,
        kInstalled,
        kNotInstalled
    };
    enum class SyncType {
        kInstalling,
        kInstallSuccess,
        kInstallFail,
        kDependPackageState,
    };

    AppItem(enum Type type, QWidget *parent = nullptr);

    void SetCategory(enum Category category);
    void SetIcon(const QString &name, const QString &backup_name = "");
    void SetName(const QString &name);
    void SetComment(const QString &comment);
    // 必须被调用，如果不调用会导致无法获取依赖包的状态。如果没有依赖包传空即可
    void SetDependPackages(const QList<QString> &packages);
    void SetUserManualName(const QString &user_manual_name);

    void SetExecConfigFilePath(const QString &path);
    void SetExec(const QList<QString> &exec);

    void SetDBusUuid(const QString &uuid);
    void SetDBusOnline(bool online);
    void SetDBusAppName(const DBusAppName &dbus_app_name);
    void SetDBusAppDesc(const DBusAppDesc &dbus_app_desc);

    QString GetToolName();

Q_SIGNALS:
    void sig_DataSync(tool_box::AppItem::SyncType sync_type, QVariant data = QVariant());

public Q_SLOTS:
    void on_DataSync(tool_box::AppItem::SyncType sync_type, QVariant data);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private Q_SLOTS:
    AppItem::DependPackageState GetDependPackageState();
    void on_InstallPackages();
    void on_ShowUserManual() const;
    void on_AddToDesktopShortcut();
    void on_DBusAppFinished(QString uuid, QString data);

private:
    bool IsPackageInstalled(const QString &package_name) const;
    void InstallStatusChanged(qint32 progress, QString info, QString detail) const;
    bool IsDependPackageInstalled() const;
    void InstallFinish(bool ok, QStringList package_list, QString error_result, QString error_reason);
    void StartExecApp();
    void StartDBusApp();
    void AddToDesktopShortcutForExec() const;
    void AddToDesktopShortcutForDBus() const;

    QPushButton *               icon_btn_;
    K::TextLabel *              name_label_;
    kdk::KToolButton *          help_btn_;
    kdk::KToolButton *          install_btn_;
    kdk::KToolButton *          installing_btn_;
    K::TextLabel *              comment_label_;

    enum Type                   type_;
    enum Category               category_;
    enum IconType               icon_type_;
    QList<QString>              icon_;
    QString                     user_manual_name_;
    QList<QString>              depend_packages_;          // 依赖包
    AppItem::DependPackageState depend_packages_state_;    // 依赖包状态
    std::future<bool>           share_future_;             // 依赖包检测结果 future 对象

    // Type::kExec 类型
    QString                     config_file_;
    QList<QString>              exec_;    // eg: {"/usr/bin/ls", "-l"}

    // Type::kDBus 类型
    QString                     uuid_;
    bool                        online_;
    DBusAppName                 dbus_app_name_;
    DBusAppDesc                 dbus_app_desc_;
};

}

Q_DECLARE_METATYPE(tool_box::AppItem::SyncType)

#endif
