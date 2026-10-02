#ifndef KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_EXAMPLE_H
#define KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_EXAMPLE_H

#include "kom_tray_interface.h"

namespace example {

class Protocol : public KomTrayInterface {
    ~Protocol();
    std::string name() override;
    std::string icon() override;
    std::string tooltip() override;
    void trigger() override;
    std::vector<KomActionEntry> actions() override;

    bool action1();
    bool action4();
};

class ProtocolProvider : public KomTrayProvider {
public:
    KomTrayInterface *create() const override;
};

}

#endif
