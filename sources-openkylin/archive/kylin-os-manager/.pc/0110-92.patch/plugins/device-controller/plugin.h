#ifndef PLUGINS_DRIVER_CONTROLLER_PLUGIN_H
#define PLUGINS_DRIVER_CONTROLLER_PLUGIN_H

#include "kom_application_interface.h"

namespace DriverControllerPlugin
{

class DriverControllerPlugin : public KomApplicationInterface
{
public:
    virtual ~DriverControllerPlugin();
    virtual std::string name() override;
    virtual std::string i18nName() override;
    virtual std::string icon() override;
    virtual int sort() override;
    virtual QWidget *createWidget() override;
};

    class DriverControllerPluginProvider : public KomApplicationProvider
    {
    public:
        DriverControllerPlugin *create() const;
};

} // namespace ServiceSupportPlugin

#endif //PLUGINS_DRIVER_CONTROLLER_PLUGIN_H
