#include "plugin.h"
#include <QObject>
#include <QDebug>
#include <QTranslator>
#include <QApplication>
#include "./src/mainwidget.h"


namespace DriverControllerPlugin {
    DriverControllerPlugin::~DriverControllerPlugin() {}


    std::string DriverControllerPlugin::name() {
        return "DeviceManager";
    }

    std::string DriverControllerPlugin::i18nName() {
        return QApplication::tr("DeviceManager").toStdString();
    }

    std::string DriverControllerPlugin::icon() {
        return "network-server-symbolic";
    }

    int DriverControllerPlugin::sort() {
        return 4;
    }

    QWidget *DriverControllerPlugin::createWidget() {
        QString tranPath("/usr/share/kylin-os-manager/translations");
        QTranslator *translator = new QTranslator;
        if (translator->load(QLocale(), "kylin-os-manager-device-controller", "_", tranPath)) {
            QApplication::installTranslator(translator);
        } else {
            qWarning() << "ProblemFeedback load translation file fail !";
        }

        MainWidget *widget = new MainWidget;
        return widget;
    }

    DriverControllerPlugin *DriverControllerPluginProvider::create() const {
        return new DriverControllerPlugin();
    }

}
