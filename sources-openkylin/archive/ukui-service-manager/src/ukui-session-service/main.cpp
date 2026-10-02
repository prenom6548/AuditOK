#include <QGuiApplication>

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

#include "sessionservicelauncher.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    QCommandLineParser parser;
    auto helpOption = parser.addHelpOption();

    QCommandLineOption pluginPathOption(QStringList() << "p" << "plugin-path");
    pluginPathOption.setValueName(QObject::tr("plugin path"));
    pluginPathOption.setDescription(QObject::tr("Specific the service plugin path where should be loaded."));
    QCommandLineOption spaceNameOption(QStringList() << "n" << "service-space");
    spaceNameOption.setValueName(QObject::tr("space name"));
    spaceNameOption.setDescription(QObject::tr("Specific the process with a name which service should be running with."));
    QCommandLineOption residentOption(QStringList() << "r" << "resident");
    residentOption.setValueName(QObject::tr("resident"));
    residentOption.setDescription(QObject::tr("Specific wether the services should be runned residently, default is yes."));
    residentOption.setDefaultValue("yes");
    QCommandLineOption servicesOption(QStringList() << "s" << "services");
    servicesOption.setValueName(QObject::tr("services"));
    servicesOption.setDescription(QObject::tr("Specific the services to be registered, splited with ';'."));
    QCommandLineOption argsOption(QStringList() << "a" << "args");
    argsOption.setValueName(QObject::tr("service args"));
    argsOption.setDescription(QObject::tr("Specific the service arguments to be processed, splited with whitespace, this option must be set after other options."));

    parser.addOption(pluginPathOption);
    parser.addOption(spaceNameOption);
    parser.addOption(residentOption);
    parser.addOption(servicesOption);
    parser.addOption(argsOption);

    if (!parser.parse(a.arguments())) {
        qWarning() << parser.errorText();
        return -1;
    }

    if (parser.isSet(helpOption)) {
        parser.showHelp();
        return 0;
    }

    QString spaceName = "Default";
    if (parser.isSet(spaceNameOption)) {
        QString tmp = parser.value(spaceNameOption);
        if (!tmp.isEmpty()) {
            spaceName = tmp;
        }
    }

    QStringList args = QStringList();
    if (parser.isSet(argsOption)) {
        args = parser.values(argsOption);
    }

    SessionServiceLauncher launcher;
    QString servicePath = "/org/ukui/ServiceSpace/" + spaceName;
    QString serviceInterface = "org.ukui.ServiceSpace." + spaceName;
    QDBusConnection::sessionBus().registerObject(servicePath, serviceInterface, &launcher, QDBusConnection::ExportAllSlots);

    bool resident = true;
    if (parser.isSet(residentOption)) {
        QString result = parser.value(residentOption).toLower();
        resident = (result == "yes" || result == "y" || result == "true");
    }

    QString pluginPath;
    if (parser.isSet(pluginPathOption)) {
        pluginPath = parser.value(pluginPathOption);
    }

    QStringList serviceNames;
    if (parser.isSet(servicesOption)) {
        serviceNames = parser.values(servicesOption);
    }

    // 注册空间服务
    QString spaceServiceName = QString("org.ukui.Service.%1").arg(spaceName);
    if (!QDBusConnection::sessionBus().registerService(spaceServiceName)) {
        qWarning() << "服务" << spaceServiceName << "已经被注册，尝试使用dbus访问？";
        return -1;
    }

    if (pluginPath.isEmpty() || serviceNames.isEmpty()) {
        qDebug() << "启动时未加载服务，参数为：" << a.arguments();
    } else {
        launcher.launchServices(pluginPath, serviceNames, resident, args);
    }

    return a.exec();
}
