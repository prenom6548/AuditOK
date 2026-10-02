#include "demoservice.h"

#include <syslog.h>

DemoService::DemoService(QObject *parent)
    : UKUI::ServiceObject{parent}
{

}

void DemoService::test()
{
    syslog(LOG_DEBUG, "ukui service demo test");

    emit called();
}

int DemoService::testGetInt()
{
    emit called();

    return 1;
}
