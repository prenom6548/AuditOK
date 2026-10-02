#include "plugin.h"
#include "./UI/uimainpage.h"
#include <QObject>
#include <QDebug>
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
    return 0;
}

QWidget *ServiceSupportPlugin::createWidget()
{
    UIMainPage *widget = UIMainPage::getInstance();
    registCallback(widget->komCallBack);
    return widget;
}

ServiceSupportPlugin *ServiceSupportPluginProvider::create() const
{
    return new ServiceSupportPlugin();
}

} // namespace ServiceSupportPlugin
