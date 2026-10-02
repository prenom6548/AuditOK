#ifndef UKUISMSERVERDBUSADAPTOR_H
#define UKUISMSERVERDBUSADAPTOR_H

#include "ukuismserver.h"
#include <QObject>
#include <QDBusAbstractAdaptor>

class ukuismserverdbusadaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukuismserver")
public:
    ukuismserverdbusadaptor(UKUISMServer *server);
    ~ukuismserverdbusadaptor();

    inline UKUISMServer *parent() const
    { return static_cast<UKUISMServer*>(QObject::parent()); }

public slots:
    bool closeSession();

    bool isCloseSession();

    void restoreSession();

signals:
     void UKUISMServerStartFinished();
};

#endif // UKUISMSERVERDBUSADAPTOR_H
