#ifndef APTINSTALLER_H
#define APTINSTALLER_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

class AptInstaller : public QObject {
Q_OBJECT
public:
    explicit AptInstaller(QString driver, QObject *parent = nullptr);

    ~AptInstaller();

    void startWorker() {
        if (m_thread != nullptr)
            m_thread->start();
    }

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
    QString m_driverInfo;
    QThread *m_thread = nullptr;
    QTimer *m_timer = nullptr;

    void processPkg();

    void installPackage(QStringList packageName);

};

#endif // APTINSTALLER_H
