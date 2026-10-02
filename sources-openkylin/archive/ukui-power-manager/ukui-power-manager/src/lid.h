#ifndef LID_H
#define LID_H

#include "common.h"


class Lid : public QObject
{
    Q_OBJECT
public:
    explicit Lid(QObject *parent = Q_NULLPTR);
    ~Lid();
    bool readLidState();
private:
    int inhibitSystemdLogin();
    int initLidConnection();

    QDBusUnixFileDescriptor logindLock;
    bool m_lidIsClosed;

private slots:
    void dealLidWatcherMssage(bool);
    void dealWakeUpSignal(bool);

signals:
    void lidChanged(bool);
};

#endif // LID_H
