#ifndef SESSION_SERVICE_APTINSTALLER_H
#define SESSION_SERVICE_APTINSTALLER_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QTimer>

static const QString KUM_DEST = "com.kylin.systemupgrade";
static const QString KUM_PATH = "/com/kylin/systemupgrade";
static const QString KUM_IFACE = "com.kylin.systemupgrade.interface";


class AptInstaller : public QObject {
Q_OBJECT
public:
    explicit AptInstaller(QString driver, QObject *parent = nullptr);

    ~AptInstaller();

    void startWorker() {
        if (m_thread != nullptr)
            m_thread->start();
    }
    QString m_driverInfo;

    QString getDriverInfo();

Q_SIGNALS:

    void installEnd();

    void finished();

    void succeed();

    void failed(QString err);

    void error(QString err);

    void progressChanged(int progess);

private slots:

    void onInstalldebStatusChanged(int, QString, QString);

    void onRecvApt(bool, QStringList, QString, QString);

private:
    QThread *m_thread = nullptr;
    QTimer *m_timer = nullptr;

    void processPkg();

    void installPackage(QStringList packageName);
};

#endif // SESSION_SERVICE_APTINSTALLER_H
