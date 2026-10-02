#ifndef KYLIN_OS_MANAGER_SRC_APPLICATION_EXAMPLE_PROTOCOL_H
#define KYLIN_OS_MANAGER_SRC_APPLICATION_EXAMPLE_PROTOCOL_H

#include "kom_application_interface.h"

namespace application_example {

class Protocol : public KomApplicationInterface {
public:
    virtual ~Protocol();
    virtual std::string name() override;
    virtual std::string i18nName() override;
    virtual std::string icon() override;
    virtual int sort() override;
    virtual QWidget *createWidget() override;
};

class ProtocolProvider : public KomApplicationProvider {
public:
    KomApplicationInterface *create() const override;
};

}

#endif
