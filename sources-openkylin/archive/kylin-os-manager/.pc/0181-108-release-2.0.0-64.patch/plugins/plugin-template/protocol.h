#ifndef KYLIN_OS_MANAGER_PLUGINS_PLUGIN_TEMPLATE_PROTOCOL_H
#define KYLIN_OS_MANAGER_PLUGINS_PLUGIN_TEMPLATE_PROTOCOL_H

#include "kom_application_interface.h"

namespace application_example {

class Protocol: public KomApplicationInterface {
	virtual ~Protocol();
	virtual std::string name() override;
    virtual std::string i18nName() override;
    virtual QWidget *createWidget() override;
	virtual std::string icon() override;
	virtual int sort() override;
};

class ProtocolProvider: public KomApplicationProvider {
public :
	KomApplicationInterface *create() const override;
};

}

#endif
