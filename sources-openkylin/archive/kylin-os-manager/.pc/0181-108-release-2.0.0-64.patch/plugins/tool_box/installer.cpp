#include "installer.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>

namespace K {

Installer &Installer::Instance() {
    static Installer installer;
    return installer;
}

Installer::Installer()
    : state_(Installer::State::kStop)
{
    QDBusConnection::systemBus().connect("com.kylin.systemupgrade",
                                         "/com/kylin/systemupgrade",
                                         "com.kylin.systemupgrade.interface",
                                         "InstalldebStatusChanged",
                                         this,
                                         SLOT(on_InstallStatusChanged(int,QString,QString)));
    QDBusConnection::systemBus().connect("com.kylin.systemupgrade",
                                         "/com/kylin/systemupgrade",
                                         "com.kylin.systemupgrade.interface",
                                         "UpdateInstallFinished",
                                         this,
                                         SLOT(on_InstallFinish(bool,QStringList,QString,QString)));
    connect(this, &Installer::sig_ConsumerInstallTask, this, &Installer::on_ConsumerInstallTask, Qt::QueuedConnection);
}

void Installer::on_InstallStatusChanged(qint32 progress, QString info, QString detail) const {
    qInfo () << "Tool box install package interface install status change:" << "\n"
             << "\tprogress: " << progress << "\n"
             << "\tinfo: " << info << "\n"
             << "\tdetail: " << detail;
    if (current_install_task_.state_changed_callback) {
        current_install_task_.state_changed_callback(progress, info, detail);
    }
}

void Installer::on_InstallFinish(bool ok, QStringList package_list, QString error_result, QString error_reason) {
    // openkylin 上安装接口的安装完成信号中 package_list 为空，所以在 openkylin 上注释掉包列表的判断逻辑
    //if (package_list != current_install_task_.package_list) {
    //    qWarning() << "Tool box install package interface finish package list is inconsisitent with the current package list."
    //               << "[ " << package_list << " ] ---> [ " << current_install_task_.package_list << " ]";
    //    return;
    //}

    qInfo () << "Tool box install package interface install finish: " << package_list;

    if (current_install_task_.finish_callback) {
        current_install_task_.finish_callback(ok, package_list, error_result, error_reason);
    }
    current_install_task_.Clear();
    Q_EMIT sig_ConsumerInstallTask();
}

void Installer::Install(const QList<QString> package_list,
                        std::function<void(qint32, QString, QString)> status_changed_callback,
                        std::function<void(bool, QList<QString>, QString, QString)> finish_callback)
{
    InstallTask task;
    task.package_list = package_list;
    task.state_changed_callback = status_changed_callback;
    task.finish_callback = finish_callback;

    {
        const std::lock_guard<std::mutex> lock(task_mutex_);
        install_tasks_.push_back(task);
        if (state_ == Installer::State::kStop) {
            Q_EMIT sig_ConsumerInstallTask();
        }
    }
}

void Installer::on_ConsumerInstallTask() {
    qInfo() << "Tool box start consume install task";
    {
        const std::lock_guard<std::mutex> lock(task_mutex_);
        if (install_tasks_.empty()) {
            state_ = Installer::State::kStop;
            qInfo() << "Tool box install cache is empty, consumer stop";
            return;
        }
        state_ = Installer::State::kRunning;
        current_install_task_ = install_tasks_.takeFirst();
    }

    qInfo() << "Tool box consume install task: " << current_install_task_.package_list;

    auto message = QDBusMessage::createMethodCall("com.kylin.systemupgrade", "/com/kylin/systemupgrade", "com.kylin.systemupgrade.interface", "InstallPackages");
    message.setArguments(QList<QVariant>{QStringList(current_install_task_.package_list)});
    auto pend = QDBusConnection::systemBus().asyncCall(message);
    auto *watcher = new QDBusPendingCallWatcher(pend);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        if (watcher->isError() || !watcher->isValid()) {
            qCritical() << "Tool box install package share object return value is error.";
            return;
        }

        QDBusPendingReply<qint32, QString> reply = *watcher;
        if (reply.isError() || !reply.isValid()) {
            qCritical() << "Tool box install package return value is not vaild.";
            return;
        }
        if (reply.count() != 2) {
            qCritical() << "Tool box install package return value count error";
            return;
        }

        auto code = reply.argumentAt<0>();
        auto desc = reply.argumentAt<1>();
        if (code ||
            // 在 2203 系统上该接口返回的第一个值即退出码为 bool 类型，后续版本改成了 int 类型
            // 当返回值为 false 时，转成 int 后为 0，所以当返回值为 0 时需要追加对错误原因的判断，来确定是否安装成功
            desc.contains("In the process of updating or Upgrading"))
        {
            // 接口调用失败
            if (current_install_task_.finish_callback) {
                current_install_task_.finish_callback(false, QList<QString>(), "", desc);
            }
            current_install_task_.Clear();
            Q_EMIT sig_ConsumerInstallTask();
        }
        qInfo() << "Tool box install package interface return. code: " << code << " desc: " << desc;
    });
}

}
