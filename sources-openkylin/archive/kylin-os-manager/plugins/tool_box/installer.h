#ifndef K_INSTALLER_H
#define K_INSTALLER_H

#include <functional>
#include <mutex>
#include <QObject>
#include <QList>
#include <QString>

namespace K {

class Installer: public QObject {
    Q_OBJECT
public:
    static Installer &Instance();
    void Install(const QList<QString> package_list,
                 std::function<void(qint32, QString, QString)> status_changed_callback,
                 std::function<void(bool, QList<QString>, QString, QString)> finish_callback);

Q_SIGNALS:
    void sig_ConsumerInstallTask();

private Q_SLOTS:
    void on_InstallStatusChanged(qint32 progress, QString info, QString detail) const;
    void on_InstallFinish(bool ok, QStringList package_list, QString error_result, QString error_reason);
    void on_ConsumerInstallTask();

private:
    enum class State {
        kRunning,
        kStop
    };

    struct InstallTask {
        void Clear() {
            package_list.clear();
            state_changed_callback = nullptr;
            finish_callback = nullptr;
        }
        QList<QString> package_list;
        std::function<void(qint32, QString, QString)> state_changed_callback;
        std::function<void(bool, QList<QString>, QString, QString)> finish_callback;
    };

    Installer();

    Installer::State state_;
    QList<InstallTask> install_tasks_;
    InstallTask current_install_task_;
    std::mutex task_mutex_;
};

}

#endif
