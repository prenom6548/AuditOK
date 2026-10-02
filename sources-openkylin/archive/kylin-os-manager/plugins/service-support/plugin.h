#ifndef PLUGINS_PLUGIN_TEMPLATE_PLUGIN_H_
#define PLUGINS_PLUGIN_TEMPLATE_PLUGIN_H_

#include "kom_application_interface.h"

namespace ServiceSupportPlugin
{

class ServiceSupportPlugin : public KomApplicationInterface
{
public:
    virtual ~ServiceSupportPlugin();
    virtual std::string name() override;
    virtual std::string i18nName() override;
    virtual std::string icon() override;
    virtual int sort() override;
    virtual QWidget *createWidget() override;
    virtual void communication(enum CommunicationType communication_type, QVariant data) override;
    virtual void init(void (*frame_callback)(const char *funcName, ...)) override;
};

class ServiceSupportPluginProvider : public KomApplicationProvider
{
public:
    ServiceSupportPlugin *create() const;
};

} // namespace ServiceSupportPlugin

#endif
