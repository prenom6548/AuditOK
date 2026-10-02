#ifndef KYLIN_OS_MANAGER_PLUGINS_SERVICE_SUPPORT_UI_SERVICE_SUPPORT_BASE_H
#define KYLIN_OS_MANAGER_PLUGINS_SERVICE_SUPPORT_UI_SERVICE_SUPPORT_BASE_H

#include <QDebug>

class ServiceSupportBase {
public:
    virtual bool GoBack() {
        return false;
    }
};

#endif

