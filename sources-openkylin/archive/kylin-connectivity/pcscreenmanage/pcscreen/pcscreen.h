#ifndef PCSCREEN_H
#define PCSCREEN_H

#include <QObject>
#include <QProcess>

class PCScreen : public QObject
{
    Q_OBJECT
public:
    explicit PCScreen(QObject *parent = nullptr);
    ~PCScreen();

    void startService();

private:
    void getRoomId();

Q_SIGNALS:
    void sigStartedScreen(QString);
    void sigStartError();

private Q_SLOTS:

private:
    QProcess *m_process = nullptr;
    QString m_roomid = "";
    bool isNormalExit = false; // 是否正常退出
};

#endif // PCSCREEN_H