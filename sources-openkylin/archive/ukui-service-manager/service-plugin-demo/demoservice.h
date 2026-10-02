#ifndef DEMOSERVICE_H
#define DEMOSERVICE_H

#include <QObject>

#include "../ukuiserviceinterface/serviceobject.h"

class DemoService : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.service.demo")
public:
    explicit DemoService(QObject *parent = nullptr);

signals:
    void called();

public slots:
    void test();

    int testGetInt();
};

#endif // DEMOSERVICE_H
