#include "plugin.h"
#include "./UI/uimainpage.h"
#include <QObject>
#include <QDebug>
#include "frame.h"
namespace ServiceSupportPlugin
{

ServiceSupportPlugin::~ServiceSupportPlugin() {}


std::string ServiceSupportPlugin::name()
{
    return "ServiceSupport";
}

std::string ServiceSupportPlugin::i18nName()
{
    return QObject::tr("ServiceSupport").toStdString();
}

std::string ServiceSupportPlugin::icon()
{
    return ":/res/ukui-service-support-symbolic.svg";
}

int ServiceSupportPlugin::sort()
{
    return 5;
}

void ServiceSupportPlugin::init(void (*frame_callback)(const char *, ...)) {
    Frame::setFrameCallback(frame_callback);
}

QWidget *ServiceSupportPlugin::createWidget()
{
    UIMainPage *widget = UIMainPage::getInstance();
    registCallback(widget->komCallBack);
    return widget;
}

void ServiceSupportPlugin::communication(CommunicationType communication_type, QVariant data)
{
    switch (communication_type) {
    case CommunicationType::kBackButtonClicked:
        UIMainPage::getInstance()->onBackButtonClick();
        break;
    case CommunicationType::kDisplayServiceSupportMainpage:
        UIMainPage::getInstance()->showMainPage();
        break;
    default:
        break;
    }
}

ServiceSupportPlugin *ServiceSupportPluginProvider::create() const
{
    return new ServiceSupportPlugin();
}

} // namespace ServiceSupportPlugin
