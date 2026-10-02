#include <iostream>

#include "protocol.h"

namespace example {

Protocol::~Protocol() {
    std::cout << "Tray3 destruction exec." << std::endl;
}

std::string Protocol::name() {
    return "Tray3";
}

std::string Protocol::icon() {
    return "kylin-os-manager";
    //return "ukui-eco-symbolic";
    //return "computer-symbolic";
    //return "kylin-ipmsg";
}

std::string Protocol::tooltip() {
    return "第三个托盘";
}

void Protocol::trigger() {
    std::cout << "kylin os manager Tray3 trigger exec." << std::endl;
}

std::vector<KomActionEntry> Protocol::actions() {
    std::vector<KomActionEntry> entry {
        {"action1", "第一个 action", std::function<bool()>(std::bind(&Protocol::action1, this))},
        {"action2", "第二个 action", std::function<bool()>(), "ukui-eco-symbolic"},
        {"action3", "",  std::function<bool()>(), "", "", KomActionType::SEPARATORS},
        {"action4", "第四个 action", std::function<bool()>(std::bind(&Protocol::action4, this)), "computer-symbolic", "第个 action", KomActionType::CHECK}
    };

    return entry;
}

bool Protocol::action1() {
    std::cout << "kylin os manager Tray3 action1 cliecked." << std::endl;
    return true;
}

bool Protocol::action4() {
    static bool result = false;
    result = !result;
    std::cout << "kylin os manager Tray3 action4 cliecked. result: " << result << std::endl;
    return result;
}

KomTrayInterface *ProtocolProvider::create() const {
    return new Protocol();
}

}
