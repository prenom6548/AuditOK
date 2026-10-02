#include "demoserviceplugin.h"
#include "demoservice.h"
#include "demoservice2.h"

#include <QDebug>
#include <QVariant>

DemoServicePlugin::DemoServicePlugin(QObject *parent)
    : QObject(parent), ServiceInterface()
{
}

QList<UKUI::ServiceObject *> DemoServicePlugin::getServiceObjects()
{
    // 获取参数
    qDebug() << "demo plugin getServiceObjects";
    if (property("arguments").isValid()) {
        qDebug() << "get arguments" << property("arguments").toStringList();
    } else {
        qDebug() << "no arguments found";
    }

    QList<UKUI::ServiceObject *> services;
    auto service1 = new DemoService;
    services << service1;

    auto service2 = new DemoService2;
    services << service2;

    return services;
}
