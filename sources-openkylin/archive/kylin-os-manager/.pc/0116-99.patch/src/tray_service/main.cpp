#include <QApplication>
#include <QDBusConnection>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include "daemon.h"
#include "client_dbus.h"
#include "service_dbus.h"
#include "../config.h"

enum class CommandType {
    DAEMON,
    START,
    STOP,
    STATUS,
    HELP,
    VERSION
};

struct CommandLineArgument {
    CommandType type;
    QString trayName;
};

bool parseCommandLine(QCommandLineParser &parser, CommandLineArgument &argument, QString &errorMessage) {
    parser.setApplicationDescription("Kylin os manager tray service provides tray management functions.");
    const QCommandLineOption help = parser.addHelpOption();
    const QCommandLineOption version = parser.addVersionOption();
    parser.addPositionalArgument("Operate", "Operate Performed.");
    parser.addPositionalArgument("Name", "Tray Name.");
    if (!parser.parse(QApplication::arguments())) {
        errorMessage = "ERROR: argument parser fail.";
        return false;
    }

    if (parser.isSet(help)) {
        argument.type = CommandType::HELP;
        return true;
    }
    if (parser.isSet(version)) {
        argument.type = CommandType::VERSION;
        return true;
    }

    QStringList positionalArguments = parser.positionalArguments();
    if (!positionalArguments.isEmpty()) {
        QString type = positionalArguments.first();
        if (type == "start" && positionalArguments.size() >= 2) {
            argument.type = CommandType::START;
            argument.trayName = positionalArguments.at(1);
            return true;
        }
        if (type == "stop" && positionalArguments.size() >= 2) {
            argument.type = CommandType::STOP;
            argument.trayName = positionalArguments.at(1);
            return true;
        }
        if (type == "status") {
            argument.type = CommandType::STATUS;
            if (positionalArguments.size() >= 2)
                argument.trayName = positionalArguments.at(1);
            return true;
        }
        errorMessage = "command line format error.";
        return false;
    }

    argument.type = CommandType::DAEMON;
    return true;
}

int runClient(CommandType type, QString name) {
    tray::DBusClient client;
    int status = 0;

    switch (type) {
    case CommandType::START:
        status = client.startTray(name);
        break;
    case CommandType::STOP:
        status =client.stopTray(name);
        break;
    case CommandType::STATUS:
        status = client.trayStatus(name);
        break;
    default:
        break;
    }

    return status;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("kylin-os-manager-tray");
    app.setApplicationVersion(KYLIN_OS_MANAGER_VERSION);

    QCommandLineParser parser;
    CommandLineArgument commandLineArgument;
    QString errorMessage;
    if (!parseCommandLine(parser, commandLineArgument, errorMessage)) {
        qCritical() << errorMessage;
        return -1;
    }
    if (commandLineArgument.type == CommandType::HELP) {
        parser.showHelp();
        Q_UNREACHABLE();
    }
    if (commandLineArgument.type == CommandType::VERSION) {
        parser.showVersion();
        Q_UNREACHABLE();
    }
    if (commandLineArgument.type != CommandType::DAEMON) {
        return runClient(commandLineArgument.type, commandLineArgument.trayName);
    }

    tray::Daemon daemon;
    daemon.enrollTrays();

    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService("com.kylin.kom.tray")) {
        qCritical() << "register dbus service fail: " << conn.lastError().message();
        return -1;
    }

    QObject object;
    tray::DBusService dbusService(&object, daemon);
    if (!conn.registerObject("/com/kylin/kom/tray", &dbusService, QDBusConnection::ExportAllSlots)) {
        qCritical() << "register dbus object fail: " << conn.lastError().message();
        return -1;
    }

    app.exec();
}