#include "demoservice2.h"
#include <QVariant>

DemoService2::DemoService2(QObject *parent)
    : UKUI::ServiceObject{parent}
{
    setProperty("demo2sub", QVariant::fromValue(this));
    setProperty("demo2sub2", QVariant::fromValue(this));
}

void DemoService2::test2()
{
    emit called2();
}

int DemoService2::testGetInt2()
{
    emit called2();

    return 2;
}
