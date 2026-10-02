#ifndef USBHOTPLUGSERVER_H
#define USBHOTPLUGSERVER_H

#include <QObject>
#include <QtDBus/QDBusServiceWatcher>
#include <QDBusContext>

class UsbHotplugServer : public QObject, protected QDBusContext {
Q_OBJECT
public:
    UsbHotplugServer(QObject *parent = nullptr);

public slots:

    void emitSingal(bool plugin, const QString devString);

    void installSucceed(const QString driverinfo);
};


#endif //USBHOTPLUGSERVER_H
