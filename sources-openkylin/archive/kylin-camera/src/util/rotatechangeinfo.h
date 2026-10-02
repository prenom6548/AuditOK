#ifndef ROTATECHANGEINFO_H
#define ROTATECHANGEINFO_H

#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <unistd.h>
#include <sys/types.h>
#include <QScreen>
#include <QPair>

static const QString KYLIN_ROTATION_PATH = "/";

static const QString KYLIN_ROTATION_SERVICE = "com.kylin.statusmanager.interface";

static const QString KYLIN_ROTATION_INTERFACE = "com.kylin.statusmanager.interface";

class RotateChangeInfo : public QObject
{
    Q_OBJECT
public:
    explicit RotateChangeInfo(QObject *parent = nullptr);
    QPair<int,int> getCurrentScale();
    bool getCurrentMode();
private:
    int m_screenWidth;
    int m_screenHeight;
    bool m_isPCMode;

Q_SIGNALS:
    void rotationChanged(bool mode,int scale_W,int scale_H);
public Q_SLOTS:
    void rotateChanged();
    void modeChanged(bool isPadMode);

};

#endif // ROTATECHANGEINFO_H
