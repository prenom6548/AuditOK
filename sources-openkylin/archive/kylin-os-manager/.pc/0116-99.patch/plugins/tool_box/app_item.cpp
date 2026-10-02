#include "app_item.h"
#include <unistd.h>
#include <sys/stat.h>
#include <functional>
#include <chrono>
#include <fstream>
#include <glib.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <kmessagebox.h>
#include "frame.h"
#include "user_experience_dialog.h"
#include "kom_buriedpoint.h"
#include "installer.h"
#include "defines.h"

namespace tool_box {

AppItem::AppItem(enum Type type, QWidget *parent)
    : QWidget(parent),
      icon_btn_(new QPushButton(this)),
      name_label_(new K::TextLabel(this)),
      help_btn_(new kdk::KToolButton(this)),
      install_btn_(new kdk::KToolButton(this)),
      installing_btn_(new kdk::KToolButton(this)),
      comment_label_(new K::TextLabel(this)),
      type_(type),
      category_(Category::kNone),
      icon_type_(IconType::kMain),
      icon_({"", ""}),
      depend_packages_state_(AppItem::DependPackageState::kUnknow),
      online_(false)
{
    icon_btn_->setFixedSize(48, 48);
    icon_btn_->setFlat(true);
    icon_btn_->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    name_label_->setMaximumWidth(204);
    name_label_->SetPixelSize(16);

    help_btn_->setFixedSize(30, 30);
    help_btn_->setIcon(QIcon::fromTheme("help-app-symbolic"));
    help_btn_->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    help_btn_->hide();
    connect(help_btn_, &kdk::KToolButton::clicked, this, &AppItem::on_ShowUserManual);

    install_btn_->setFixedSize(30, 30);
    install_btn_->setIcon(QIcon::fromTheme("folder-download-symbolic"));
    install_btn_->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    install_btn_->hide();
    connect(install_btn_, &kdk::KToolButton::clicked, this, &AppItem::on_InstallPackages);

    installing_btn_->setLoading(true);
    installing_btn_->hide();

    auto *name_hbox_layout = new QHBoxLayout;
    name_hbox_layout->setContentsMargins(0, 0, 0, 0);
    name_hbox_layout->setSpacing(0);
    name_hbox_layout->addWidget(name_label_);
    name_hbox_layout->addStretch();
    name_hbox_layout->addWidget(help_btn_);
    name_hbox_layout->addSpacing(2);
    name_hbox_layout->addWidget(install_btn_);
    name_hbox_layout->addWidget(installing_btn_);

    comment_label_->setFixedWidth(260);
    comment_label_->SetFontColor(QColor(143, 147, 153));

    auto *right_vbox_layout = new QVBoxLayout;
    right_vbox_layout->setContentsMargins(0, 0, 0, 0);
    right_vbox_layout->setSpacing(0);
    right_vbox_layout->addLayout(name_hbox_layout);
    right_vbox_layout->addWidget(comment_label_);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setContentsMargins(20, 20, 20, 20);
    hbox_layout->setSpacing(0);
    hbox_layout->addWidget(icon_btn_);
    hbox_layout->addSpacing(12);
    hbox_layout->addLayout(right_vbox_layout);
    setLayout(hbox_layout);
}

void AppItem::SetCategory(enum Category category) {
    category_ = category;
}

void AppItem::SetIcon(const QString &name, const QString &backup_name) {
    icon_[IconType::kMain] = name;
    icon_[IconType::kBackup] = backup_name;

    QIcon icon;
    if (name.contains('/')) {
        // 表明是一个路径
        QIcon temp_icon(name);
        icon.swap(temp_icon);
    } else {
        auto temp_icon = QIcon::fromTheme(name);
        icon.swap(temp_icon);
    }

    if (icon.isNull()) {
        // 使用备用图标
        icon_type_ = IconType::kBackup;
        if (backup_name.contains('/')) {
            QIcon temp_icon(backup_name);
            icon.swap(temp_icon);
        } else {
            auto temp_icon = QIcon::fromTheme(backup_name);
            icon.swap(temp_icon);
        }
    }

    icon_btn_->setIcon(icon);
    icon_btn_->setIconSize(QSize(48, 48));
}

void AppItem::SetName(const QString &name) {
    name_label_->SetText(name);
}

void AppItem::SetComment(const QString &comment) {
    comment_label_->SetText(comment);
}

void AppItem::SetDependPackages(const QList<QString> &packages) {
    depend_packages_ = packages;
    if (packages.empty()) {
        depend_packages_state_ = DependPackageState::kInstalled;
    } else {
        share_future_ = std::async(&AppItem::IsDependPackageInstalled, this);
    }
}

void AppItem::SetUserManualName(const QString &user_manual_name) {
    user_manual_name_ = user_manual_name;
}

void AppItem::SetExecConfigFilePath(const QString &path) {
    config_file_ = path;
}

void AppItem::SetExec(const QList<QString> &exec) {
    exec_ = exec;
}

void AppItem::SetDBusUuid(const QString &uuid) {
    uuid_ = uuid;
}

void AppItem::SetDBusOnline(bool online) {
    online_ = online;
}

void AppItem::SetDBusAppName(const DBusAppName &dbus_app_name) {
    dbus_app_name_ = dbus_app_name;

    auto language = QLocale::system().name();
    if (language == "zh_CN") {
        if (!dbus_app_name.name_zh_cn.isEmpty()) {
            name_label_->SetText(dbus_app_name.name_zh_cn);
            return;
        }
    } else if (language == "en_US") {
        if (!dbus_app_name.name_en_us.isEmpty()) {
            name_label_->SetText(dbus_app_name.name_en_us);
            return;
        }
    } else {
        if (!dbus_app_name.name_zh_cn.isEmpty()) {
            name_label_->SetText(dbus_app_name.name_zh_cn);
            return;
        }
    }
    name_label_->SetText(dbus_app_name.name);
}

void AppItem::SetDBusAppDesc(const DBusAppDesc &dbus_app_desc) {
    dbus_app_desc_ = dbus_app_desc;

    auto language = QLocale::system().name();
    if (language == "zh_CN") {
        if (!dbus_app_desc.desc_zh_cn.isEmpty()) {
            comment_label_->SetText(dbus_app_desc.desc_zh_cn);
            return;
        }
    } else if (language == "en_US") {
        if (!dbus_app_desc.desc_en_us.isEmpty()) {
            comment_label_->SetText(dbus_app_desc.desc_en_us);
            return;
        }
    } else {
        if (!dbus_app_desc.desc_zh_cn.isEmpty()) {
            comment_label_->SetText(dbus_app_desc.desc_zh_cn);
            return;
        }
    }
    comment_label_->SetText(dbus_app_desc.desc);
}

QString AppItem::GetToolName()
{
    return name_label_->text();
}

bool AppItem::IsPackageInstalled(const QString &package_name) const {
    QProcess process;
    process.setProgram("dpkg");
    QList<QString> args{"-l", package_name};
    process.setArguments(args);
    process.start();
    process.waitForFinished();

    auto output = QString::fromLocal8Bit(process.readAll());
    auto lines = output.split('\n');
    lines.removeAll("");
    if (lines.empty()) {
        return false;
    }

    auto last_line = lines.back().trimmed();
    auto fields = last_line.split(' ');
    fields.removeAll("");
    if (fields.size() < 2) {
        return false;
    }

    auto name = fields.at(1);

    return name == package_name;
}

bool AppItem::IsDependPackageInstalled() const {
    for (const auto &package: depend_packages_) {
        if (!IsPackageInstalled(package)) {
            return false;
        }
    }

    return true;
}

AppItem::DependPackageState AppItem::GetDependPackageState() {
    if (depend_packages_state_ == AppItem::DependPackageState::kUnknow &&
        share_future_.valid())
    {
        auto state = share_future_.wait_for(std::chrono::seconds(0));
        if (state == std::future_status::ready) {
            if (share_future_.get()) {
                depend_packages_state_ = AppItem::DependPackageState::kInstalled;
            } else {
                depend_packages_state_ = AppItem::DependPackageState::kNotInstalled;
            }
        }
    }
    return depend_packages_state_;
}

void AppItem::enterEvent([[maybe_unused]]QEvent *event) {
    if (!user_manual_name_.isEmpty()) {
        help_btn_->show();
    } else {
        help_btn_->hide();
    }
    if (GetDependPackageState() == AppItem::DependPackageState::kNotInstalled && installing_btn_->isHidden()) {
        install_btn_->show();
    } else {
        install_btn_->hide();
    }
}

void AppItem::leaveEvent(QEvent *event) {
    help_btn_->hide();
    install_btn_->hide();
}

void AppItem::mouseReleaseEvent(QMouseEvent *event) {
    switch (GetDependPackageState()) {
        case DependPackageState::kUnknow:
            QMessageBox::information(nullptr, "", tr("Getting installation package status, please try again later"));
            return;
        case DependPackageState::kNotInstalled:
            QMessageBox::information(nullptr, "", tr("Please proceed with installation first"));
            return;
        default:
            break;
    }
    switch (type_) {
        case Type::kExec:
            StartExecApp();
            break;
        case Type::kDBus:
            StartDBusApp();
            break;
        default:
            break;
    }
}

void AppItem::StartExecApp() {
    if (exec_.empty()) {
        qCritical() << "Start tool box app fail: exec is empty.";
        QMessageBox::critical(nullptr, "", tr("No application detected"));
        return;
    }

    QProcess process;
    process.setProgram(exec_.front());
    QList<QString> args;
    for (int i = 1; i < exec_.size(); i++) {
        args.push_back(exec_.at(i));
    }
    process.setArguments(args);

    qint64 pid{0};
    if (process.startDetached(&pid)) {
        kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kToolBox,
                                        kom::BuriedPoint::EventCode::kStartApplication,
                                        {{"appStartPath", exec_.front()}});
    } else {
        qCritical() << "Start tool box exec app " << exec_ << " fail.";

        auto ok = IsDependPackageInstalled();
        if (!ok) {
            depend_packages_state_ = DependPackageState::kNotInstalled;
            Q_EMIT sig_DataSync(SyncType::kDependPackageState, false);
            QMessageBox::information(this, "", tr("Please proceed with installation first"));
        }
    }
}

void AppItem::StartDBusApp() {
    auto message = QDBusMessage::createMethodCall(TREASUREBOX_DBUS_NAME,
                                                  TREASUREBOX_DBUS_PATH,
                                                  TREASUREBOX_DBUS_INTERFACE,
                                                  "IsAppRunning");
    message.setArguments(QList<QVariant>{uuid_});
    auto reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        qCritical() << "Tool box start d-bus app fail: check instance return error.";
        return;
    }
    auto args = reply.arguments();
    if (args.empty()) {
        qCritical() << "Tool box start d-bus app fail: check instance return null.";
        return;
    }

    auto is_running = args.front().toBool();
    auto exec_message = QDBusMessage::createMethodCall(TREASUREBOX_DBUS_NAME,
                                                       TREASUREBOX_DBUS_PATH,
                                                       TREASUREBOX_DBUS_INTERFACE,
                                                       "ExecApp");
    exec_message.setArguments(QList<QVariant>{uuid_});
    auto exec_reply = QDBusConnection::sessionBus().call(exec_message);
    if (exec_reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        qCritical() << "Tool box start d-bus app fail: exec d-bus return error.";
    } else {
        if (!is_running) {
            QDBusConnection::sessionBus().disconnect(TREASUREBOX_DBUS_NAME,
                                                     TREASUREBOX_DBUS_PATH,
                                                     TREASUREBOX_DBUS_INTERFACE,
                                                     "ExecAppFinished",
                                                     this,
                                                     SLOT(on_DBusAppFinished(QString,QString)));
            QDBusConnection::sessionBus().connect(TREASUREBOX_DBUS_NAME,
                                                  TREASUREBOX_DBUS_PATH,
                                                  TREASUREBOX_DBUS_INTERFACE,
                                                  "ExecAppFinished",
                                                  this,
                                                  SLOT(on_DBusAppFinished(QString,QString)));
        }
    }
}

void AppItem::on_DBusAppFinished(QString uuid, QString data) {
    if (uuid != uuid_) {
        return;
    }

    QDBusConnection::sessionBus().disconnect(TREASUREBOX_DBUS_NAME,
                                             TREASUREBOX_DBUS_PATH,
                                             TREASUREBOX_DBUS_INTERFACE,
                                             "ExecAppFinished",
                                             this,
                                             SLOT(on_DBusAppFinished(QString,QString)));

    qInfo() << "Tool box " << name_label_->text() << "finished: " << data;

    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(data.toLocal8Bit(), &error);
    if (doc.isNull()) {
        qCritical() << "Tool box d-bus app finished data parse fail: " << error.errorString();
        return;
    }
    if (!doc.isObject()) {
        qCritical() << "Tool box d-bus app finished data format error.";
        return;
    }

    const auto obj = doc.object();
    const auto exit_code_iter = obj.find("ret");
    if (exit_code_iter == obj.end()) {
        qCritical() << "Tool box d-bus app finished data not contain [code] field";
        return;
    }

    auto exit_code = exit_code_iter.value().toInt();
    if (exit_code) {
        QString desc;
        const auto desc_iter = obj.find("desc");
        if (desc_iter != obj.end()) {
            desc = desc_iter.value().toString();
        }
        qCritical() << "Tool box d-bus app return fail: " << desc;
    } else {
        // 当小工具运行成功时，弹窗收集用户体验
        if (category_ == Category::kTroubleshooting) {
            // 弹窗需要居中应用显示，此处寻找主界面的指针，将主界面设置为弹窗的父类
            QWidget *parent = this;
            QWidget *pointer = this;
            while ((pointer = pointer->parentWidget()) != nullptr) {
                if (pointer->objectName() == "main_frame_interface") {
                    parent = pointer;
                    break;
                }
            }
            K::UserExperienceDialog user_experience_dialog(parent);
            user_experience_dialog.SetFeedBackCallback([]() {
                Frame::SwitchToProblemFeedback();
            });

            auto name = name_label_->text();
            user_experience_dialog.SetFixedCallback([name, uuid]() {
                QMap<QString, QString> custom_property = {
                        {"name", name},
                        {"uuid", uuid}
                };
                kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kExperienceInformationCollection,
                                                kom::BuriedPoint::EventCode::kFaultRepairToolRepairSuccessful,
                                                custom_property);
            });

            user_experience_dialog.exec();
        }
    }
}

void AppItem::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    QAction add_desktop_shortcut(tr("Add to desktop shortcut"), this);
    connect(&add_desktop_shortcut, &QAction::triggered, this, &AppItem::on_AddToDesktopShortcut);
    menu.addAction(&add_desktop_shortcut);
    menu.exec(event->globalPos());
}

void AppItem::on_InstallPackages() {
    qInfo() << "Tool box start install package: " << depend_packages_;
    install_btn_->hide();
    installing_btn_->show();
    Q_EMIT sig_DataSync(SyncType::kInstalling);

    K::Installer::Instance().Install(depend_packages_,
                                     std::bind(&AppItem::InstallStatusChanged, this, std::placeholders::_1,
                                               std::placeholders::_2, std::placeholders::_3),
                                     std::bind(&AppItem::InstallFinish, this, std::placeholders::_1,
                                               std::placeholders::_2, std::placeholders::_3,
                                               std::placeholders::_4));
    qInfo() << "Tool box packages [ " << depend_packages_ << " ] enter installation cache";
}

void AppItem::InstallStatusChanged(qint32 progress, QString info, QString detail) const {

}

void AppItem::InstallFinish(bool ok, QStringList package_list, QString error_result, QString error_reason) {
    qInfo() << "Tool box [ " << package_list << " ] install finish: " << ok;

    installing_btn_->hide();

    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32) 0);
    args.push_back("kylin-os-manager");
    if (ok) {
        args.push_back(tr("Installed successfully"));    // 标题
        args.push_back(tr("The %1 has been successfully installed.").arg(name_label_->text()));    // 正文
        depend_packages_state_ = AppItem::DependPackageState::kInstalled;
        Q_EMIT sig_DataSync(SyncType::kInstallSuccess);
        qInfo() << "Tool box install [ " << package_list << " ] packages success.";
    } else {
        args.push_back(tr("Installation failed"));    // 标题
        args.push_back(tr("The %1 was not successfully installed. Please try again later.").arg(name_label_->text()));    // 正文
        depend_packages_state_ = AppItem::DependPackageState::kNotInstalled;
        Q_EMIT sig_DataSync(SyncType::kInstallFail);
        qCritical() << "Tool box install [ " << package_list << " ] packages fail. error result: " << error_result
                    << "error reason: " << error_reason;
    }
    args.push_back(QStringList());
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        if (!watcher->isValid() || watcher->isError()) {
            qCritical() << "Notify D-Bus share object is error.";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (!reply.isValid() || reply.isError()) {
            qCritical() << "Notify D-Bus return vaule is not valid.";
            return;
        }
    });
}

void AppItem::on_ShowUserManual() const {
    auto service_name = QString("com.kylinUserGuide.hotel_%1").arg(getuid());
    auto message = QDBusMessage::createMethodCall(service_name, "/", "com.guide.hotel", "showGuide");
    message.setArguments(QList<QVariant>{user_manual_name_});
    auto reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Tool box call user manual fail: " << reply.errorMessage();
    }
}

void AppItem::on_AddToDesktopShortcut() {
    switch (GetDependPackageState()) {
        case DependPackageState::kUnknow:
            QMessageBox::information(nullptr, "", tr("Getting installation package status, please try again later"));
            return;
        case DependPackageState::kNotInstalled:
            QMessageBox::information(nullptr, "", tr("Please proceed with installation first"));
            return;
        default:
            break;
    }
    switch (type_) {
        case Type::kExec:
            AddToDesktopShortcutForExec();
            break;
        case Type::kDBus:
            AddToDesktopShortcutForDBus();
            break;
        default:
            break;
    }
}

void AppItem::AddToDesktopShortcutForExec() const {
    if (exec_.empty()) {
        qCritical() << "Tool box add to desktop shortcut fail: exec is empty.";
        return;
    }
    auto exec_file_name = exec_.first();
    const auto index = exec_file_name.lastIndexOf('/');
    if (index > 0) {
       exec_file_name = exec_file_name.mid(index + 1);
    }

    const auto desktop_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DesktopLocation);

    // 拼接临时 desktop 隐藏文件
    QString temp_desktop_file = QString("%1/.%2.desktop").arg(desktop_path, exec_file_name);

    std::ofstream ofs(temp_desktop_file.toStdString(), std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        qCritical() << "Tool box add to desktop shortcut fail: create temp desktop file fail.";
        return;
    }

    ofs << "[Desktop Entry]\n";
    ofs << "Encoding=UTF-8\n";

    g_autoptr(GKeyFile) key_file = g_key_file_new();
    g_autoptr(GError) error = NULL;
    do {
        if (!g_key_file_load_from_file(key_file, config_file_.toStdString().c_str(), G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
            qCritical() << "Tool box add to desktop shortcut fail: load key file fail, " << error->message;
            break;
        }
        g_autofree gchar *name = g_key_file_get_string(key_file, "ToolBox Entry", "Name", NULL);
        if (name != NULL) {
            ofs << "Name=" << name << "\n";
        }
        g_autofree gchar *name_zh_cn = g_key_file_get_string(key_file, "ToolBox Entry", "Name[zh_CN]", NULL);
        if (name_zh_cn != NULL) {
            ofs << "Name[zh_CN]=" << name_zh_cn << "\n";
        }
        g_autofree gchar *name_zh_hk = g_key_file_get_string(key_file, "ToolBox Entry", "Name[zh_HK]", NULL);
        if (name_zh_hk != NULL) {
            ofs << "Name[zh_HK]=" << name_zh_hk << "\n";
        }
        g_autofree gchar *name_bo_cn = g_key_file_get_string(key_file, "ToolBox Entry", "Name[bo_CN]", NULL);
        if (name_bo_cn != NULL) {
            ofs << "Name[bo_CN]=" << name_bo_cn << "\n";
        }
        g_autofree gchar *name_mn = g_key_file_get_string(key_file, "ToolBox Entry", "Name[mn]", NULL);
        if (name_mn != NULL) {
            ofs << "Name[mn]=" << name_mn << "\n";
        }
        g_autofree gchar *comment = g_key_file_get_string(key_file, "ToolBox Entry", "Comment", NULL);
        if (comment != NULL) {
            ofs << "Comment=" << comment << "\n";
        }
        g_autofree gchar *comment_zh_cn = g_key_file_get_string(key_file, "ToolBox Entry", "Comment[zh_CN]", NULL);
        if (comment_zh_cn != NULL) {
            ofs << "Comment[zh_CN]=" << comment_zh_cn << "\n";
        }
        g_autofree gchar *comment_zh_hk = g_key_file_get_string(key_file, "ToolBox Entry", "Comment[zh_HK]", NULL);
        if (comment_zh_hk != NULL) {
            ofs << "Comment[zh_HK]=" << comment_zh_hk << "\n";
        }
        g_autofree gchar *comment_bo_cn = g_key_file_get_string(key_file, "ToolBox Entry", "Comment[bo_CN]", NULL);
        if (comment_bo_cn != NULL) {
            ofs << "Comment[bo_CN]=" << comment_bo_cn << "\n";
        }
        g_autofree gchar *comment_mn = g_key_file_get_string(key_file, "ToolBox Entry", "Comment[mn]", NULL);
        if (comment_mn != NULL) {
            ofs << "Comment[mn]=" << comment_mn << "\n";
        }
        g_autofree gchar *icon = g_key_file_get_string(key_file, "ToolBox Entry", "Icon", NULL);
        if (icon != NULL) {
            ofs << "Icon=" << icon << "\n";
        }
        g_autofree gchar *exec = g_key_file_get_string(key_file, "ToolBox Entry", "Exec", NULL);
        if (exec != NULL) {
            ofs << "Exec=" << exec << "\n";
        }
        ofs << "StartupNotify=false" << "\n";
        ofs << "Terminal=false" << "\n";
        ofs << "Type=Application" << "\n";
        ofs << "Categories=System" << "\n";
        ofs.close();

        // 修改文件权限
        if (::chmod(temp_desktop_file.toStdString().c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) {
            qCritical() << "Tool box add to desktop shortcut fail: modify temp dekstop file limits fail, " << strerror(errno);
            break;
        }

        // 修改文件名
        QString desktop_file = QString("%1/%2.desktop").arg(desktop_path, exec_file_name);
        if (::rename(temp_desktop_file.toStdString().c_str(), desktop_file.toStdString().c_str())) {
            qCritical() << "Tool box add to desktop shortcut fail: modify file name fail, " << strerror(errno);
            break;
        }

        return;
    } while (0);

    // 创建 desktop 文件失败
    if (ofs.is_open()) {
        ofs.close();
    }

    // 失败会执行此处
    // 删除临时文件
    if (::remove(temp_desktop_file.toStdString().c_str())) {
        qCritical() << "Tool box add to desktop shortcut delete temp desktop file fail: " << strerror(errno);
    }
}

void AppItem::AddToDesktopShortcutForDBus() const {
    if (uuid_.isEmpty()) {
        qCritical() << "Tool box dbus app add to desktop shortcut fail: uuid is empty.";
        return;
    }

    auto desktop_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DesktopLocation);
    QString temp_desktop_file = QString("%1/.%2.desktop").arg(desktop_path, uuid_);

    std::ofstream ofs(temp_desktop_file.toStdString(), std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        qCritical() << "Tool box dbus app add to desktop shortcur fail: create desktop file fail.";
        return;
    }

    ofs << "[Desktop Entry]\n";
    ofs << "Encoding=UTF-8\n";

    // 名字
    if (!dbus_app_name_.name_en_us.isEmpty()) {
        ofs << "Name=" << dbus_app_name_.name_en_us.toStdString() << "\n";
    } else {
        ofs << "Name=" << dbus_app_name_.name.toStdString() << "\n";
    }
    if (!dbus_app_name_.name_zh_cn.isEmpty()) {
        ofs << "Name[zh_CN]=" << dbus_app_name_.name_zh_cn.toStdString() << "\n";
        ofs << "Name[zh_HK]=" << dbus_app_name_.name_zh_cn.toStdString() << "\n";
        ofs << "Name[bo_CN]=" << dbus_app_name_.name_zh_cn.toStdString() << "\n";
        ofs << "Name[mn]=" << dbus_app_name_.name_zh_cn.toStdString() << "\n";
    } else {
        ofs << "Name[zh_CN]=" << dbus_app_name_.name.toStdString() << "\n";
        ofs << "Name[zh_HK]=" << dbus_app_name_.name.toStdString() << "\n";
        ofs << "Name[bo_CN]=" << dbus_app_name_.name.toStdString() << "\n";
        ofs << "Name[mn]=" << dbus_app_name_.name.toStdString() << "\n";
    }

    // 描述
    if (!dbus_app_desc_.desc_en_us.isEmpty()) {
        ofs << "Comment=" << dbus_app_desc_.desc_en_us.toStdString() << "\n";
    } else {
        ofs << "Comment=" << dbus_app_desc_.desc.toStdString() << "\n";
    }
    if (!dbus_app_desc_.desc_zh_cn.isEmpty()) {
        ofs << "Comment[zh_CN]=" << dbus_app_desc_.desc_zh_cn.toStdString() << "\n";
        ofs << "Comment[zh_HK]=" << dbus_app_desc_.desc_zh_cn.toStdString() << "\n";
        ofs << "Comment[bo_CN]=" << dbus_app_desc_.desc_zh_cn.toStdString() << "\n";
        ofs << "Comment[mn]=" << dbus_app_desc_.desc_zh_cn.toStdString() << "\n";
    } else {
        ofs << "Comment[zh_CN]=" << dbus_app_desc_.desc.toStdString() << "\n";
        ofs << "Comment[zh_HK]=" << dbus_app_desc_.desc.toStdString() << "\n";
        ofs << "Comment[bo_CN]=" << dbus_app_desc_.desc.toStdString() << "\n";
        ofs << "Comment[mn]=" << dbus_app_desc_.desc.toStdString() << "\n";
    }

    ofs << "Icon=" << icon_[icon_type_].toStdString() << "\n";
    ofs << "Exec=" << "/usr/share/kylin-os-manager/plugins/tool-box/bin/app_launcher " << uuid_.toStdString() << "\n";
    ofs << "StartupNotify=false" << "\n";
    ofs << "Terminal=false" << "\n";
    ofs << "Type=Application" << "\n";
    ofs << "Categories=System" << "\n";
    ofs.close();

    do {
        // 修改文件权限
        if (::chmod(temp_desktop_file.toStdString().c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) {
            qCritical() << "Tool box dbus app add to desktop shortcut fail: modify temp desktop file limits fail, " << strerror(errno);
            break;
        }
        // 修改文件名
        auto desktop_file = QString("%1/%2.desktop").arg(desktop_path, uuid_);
        if (::rename(temp_desktop_file.toStdString().c_str(), desktop_file.toStdString().c_str())) {
            qCritical() << "Tool box dbus app add to desktop shortcut fail: modify file name fail, " << strerror(errno);
            break;
        }

        // 成功
        return;
    } while (0);

    // 失败会执行此处
    // 删除临时文件
    if (::remove(temp_desktop_file.toStdString().c_str())) {
        qCritical() << "Tool box dbus app add to desktop shortcut delete temp desktop file fail: " << strerror(errno);
    }
}

void AppItem::on_DataSync(tool_box::AppItem::SyncType sync_type, QVariant data) {
    switch (sync_type) {
        case SyncType::kInstalling:
            install_btn_->hide();
            installing_btn_->show();
            break;
        case SyncType::kInstallSuccess:
            install_btn_->hide();
            installing_btn_->hide();
            depend_packages_state_ = AppItem::DependPackageState::kInstalled;
            break;
        case SyncType::kInstallFail:
            install_btn_->show();
            installing_btn_->hide();
            depend_packages_state_ = AppItem::DependPackageState::kNotInstalled;
            break;
        case SyncType::kDependPackageState:
            if (data.toBool()) {
                depend_packages_state_ = DependPackageState::kInstalled;
            } else {
                depend_packages_state_ = DependPackageState::kNotInstalled;
            }
        default:
            break;
    }
}

}
