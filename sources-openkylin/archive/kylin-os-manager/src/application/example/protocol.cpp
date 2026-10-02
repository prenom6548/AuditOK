#include "main_widget.h"
#include "protocol.h"

namespace application_example {

Protocol::~Protocol() {

}

std::string Protocol::name() {
    return "ApplicationSdkExample";
}

std::string Protocol::i18nName() {
    return "SDK控件展示";
}

std::string Protocol::icon() {
    return "ukui-full-backup-symbolic";
}

int Protocol::sort() {
    return 999;
}

QWidget *Protocol::createWidget() {
    auto *w = new MainWindow;
    return w;
}

KomApplicationInterface *ProtocolProvider::create() const {
    return new Protocol();
}

}
