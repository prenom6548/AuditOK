#ifndef UNINSTALLER_H
#define UNINSTALLER_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QTimer>

class Uninstaller : public QObject {
Q_OBJECT
public:
    explicit Uninstaller(QString driver, QObject *parent = nullptr);

    ~Uninstaller();

    void startWorker() {
        if (m_thread != nullptr)
            m_thread->start();
    }

Q_SIGNALS:

    void uninstallEnd();

    void finished();

    void succeed();

    void failed(QString err);

    void error(QString err);

    void progressChanged(int progess);

private slots:

    void onUninstalldebStatusChanged(int, QString, QString);

    void onRecvPurge(bool, QString, QString);

private:
    QString m_driverInfo;
    QThread *m_thread = nullptr;
    QTimer *m_timer = nullptr;

    void processPkg();

    void uninstallPackage(QStringList packageName);
};


#endif //UNINSTALLER_H
