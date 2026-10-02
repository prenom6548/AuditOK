#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include <QProcess>
#include <QLocalServer>
#include <QLocalSocket>

class Control : public QObject
{
    Q_OBJECT
public:
    Control(QObject *parent = nullptr);
    ~Control();

    void startServer();
    void startClient(QString address);

Q_SIGNALS:
    void sigServerState(bool);
    void sigClientState(bool);
    void sigControlState(bool);
private Q_SLOTS:
    void slotNewConnected();
    void slotReadyRead();

private:
    QProcess *m_process = nullptr;
    QLocalServer *m_server = nullptr;
    QLocalSocket *m_socket = nullptr;
    bool m_isServer = true;
};

#endif  // CONTROL_H