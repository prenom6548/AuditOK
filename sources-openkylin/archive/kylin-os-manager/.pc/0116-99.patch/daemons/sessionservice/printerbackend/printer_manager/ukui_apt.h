#ifndef UKUI_APT_H
#define UKUI_APT_H
// Use QObject
#include <QDebug>
#include <QObject>
#include <QVariant>
// Use libqapt as apt interface
//#include <qapt/backend.h>
//#include <qapt/debfile.h>
//#include <qapt/package.h>
//#include <qapt/transaction.h>
//
#include "device_monitor.h"
#include "device_information.h"
#include <QThread>

class QTimer;

enum class ukuiAptError : int {
    UKUI_APT_SUCCESS = 0,
    UKUI_APT_CANNOT_ACCESS_SERVER,
    UKUI_APT_NO_SUCH_PACKAGE,
    UKUI_APT_INSTALL_FAIL,
};

enum class ukuiInstallStatus : int {
    UKUI_INSTALL_START = 0,
    UKUI_INSTALL_IN_PROGRESS,
    UKUI_INSTALL_SUCCESS,
    UKUI_INSTALL_FAIL,
    UKUI_INSTALL_STOP,
};

const int APT_INSTALL_MAX_MILLISECOND = 60 * 1000 * 3;

//Worker Class
//TODO : 需要手动判断包是否已经安装，优化匹配速度
class AptUtilHelper : public QObject
{
    Q_OBJECT
public:
    explicit AptUtilHelper(DeviceInformation device,QObject *parent = nullptr);
    ~AptUtilHelper();
Q_SIGNALS:
    void installEnd();
    void finished();
    void succeed();
    void failed();
    void canceled();
    void error(QString err);
//暂时还没发出来
    void alreadyInstalled();
private slots:
    void onInstalldebStatusChanged(int, QString, QString);
    void onRecvApt(bool, QStringList, QString, QString);
    void stopAptTask();
    void handleCancel();
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
private:
    void processPkg();
    void installPackage(QStringList packageName);
    QThread *m_thread{nullptr};
    QTimer *m_timer{nullptr};
    QString m_packageName = "";
    DeviceInformation m_device = DeviceInformation();
    QStringList m_packages = QStringList();
    QMap<QString, QString> m_messages;
    ukuiInstallStatus m_installStatus{ukuiInstallStatus::UKUI_INSTALL_START};
};

//TODO : 需要手动判断包是否已经安装，优化匹配速度
class DebUtilHelper : public QObject
{
    Q_OBJECT
public:
    explicit DebUtilHelper(QString debName,QObject *parent = nullptr);
    ~DebUtilHelper();
Q_SIGNALS:
    void installEnd();
    void finished();
    void failed();
    void succeed();
    void error(QString err);
//暂时还没发出来
    void alreadyInstalled();
private slots:
    void onInstalldebStatusChanged(int, QString, QString);
    void onRecvApt(bool, QString, QString);
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
private:
    void processDeb();
    void installLocalDeb(QString debFilePath);
    //工作线程
    QThread *m_thread{nullptr};
    QTimer *m_timer{nullptr};
    QString m_debName = "";
    QMap<QString, QString> m_messages;
};

#endif //UKUI_APT_H
