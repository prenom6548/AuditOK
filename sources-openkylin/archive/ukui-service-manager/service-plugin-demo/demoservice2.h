#ifndef DEMOSERVICE2_H
#define DEMOSERVICE2_H

#include "../ukuiserviceinterface/serviceobject.h"

class DemoService2 : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.service.demo2")
public:
    explicit DemoService2(QObject *parent = nullptr);

signals:
    void called2();

public slots:
    void test2();

    int testGetInt2();
};

#endif // DEMOSERVICE2_H
