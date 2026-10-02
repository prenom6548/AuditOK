#include <iostream>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "conf_parser.h"
#include "daemon.h"
#include "service_dbus.h"
#include "client_dbus.h"
#include "../config.h"

enum class CommandType {
    HELP,
    VERSION,
    START,
    STOP,
    RESTART,
    STATUS,
    DAEMON
};

struct CommandLineArgs{
    CommandType type;
    QString processName;
};

bool parseCommandLineArgs(QCommandLineParser &parser, CommandLineArgs &args, QString errorMessage) {
    parser.setApplicationDescription("kylin os manager process management"
                                     " service is used to provide management services for its child processes.");
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();
    parser.addPositionalArgument("Operate", "Operate Performed.");
    parser.addPositionalArgument("Name", "Process Name.");
    if (!parser.parse(QCoreApplication::arguments())) {
        errorMessage = "Parse CommandLine arg fail.";
        return false;
    }

    if (parser.isSet(helpOption)) {
        args.type = CommandType::HELP;
        return true;
    }
    if (parser.isSet(versionOption)) {
        args.type = CommandType::VERSION;
        return true;
    }

    QStringList positionalArgs = parser.positionalArguments();
    if (!positionalArgs.isEmpty()) {
        if (positionalArgs.size() >= 2) {
            QString operate = positionalArgs.at(0);
            if (operate == "start")
                args.type = CommandType::START;
            else if (operate == "stop")
                args.type = CommandType::STOP;
            else if (operate == "restart")
                args.type = CommandType::RESTART;
            else if (operate == "status")
                args.type = CommandType::STATUS;
            args.processName = positionalArgs.at(1);
            return true;
        } else {
            if (positionalArgs.first() == "status") {
                args.type = CommandType::STATUS;
                args.processName = "";
                return true;
            }
            errorMessage = "CommandLine format error.";
            return false;
        }
    }

    args.type = CommandType::DAEMON;
    return true;
}

void runClient(CommandType type, const QString &name) {
    process_manager::DBusClient client;
    switch (type) {
    case CommandType::START:
        client.managerProcess(process_manager::DBusClient::Operate::START, name);
        break;
    case CommandType::STOP:
        client.managerProcess(process_manager::DBusClient::Operate::STOP, name);
        break;
    case CommandType::RESTART:
        client.managerProcess(process_manager::DBusClient::Operate::RESTART, name);
        break;
    case CommandType::STATUS:
        client.status(name);
    default:
        break;
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("kylin-os-manager-process-manager");
    QCoreApplication::setApplicationVersion(KYLIN_OS_MANAGER_VERSION);

    QCommandLineParser commandParser;
    CommandLineArgs commandLineArgs;
    QString errorMessage;
    if (!parseCommandLineArgs(commandParser, commandLineArgs, errorMessage)) {
        std::cout << "Parsing command line failed. ErrorMessage: " << errorMessage.toStdString() << std::endl;
        return -1;
    }
    if (commandLineArgs.type == CommandType::HELP) {
        // 该方法会显示帮助信息并退出进程
        commandParser.showHelp(0);
        Q_UNREACHABLE();
    }
    if (commandLineArgs.type == CommandType::VERSION) {
        // 该方法会显示帮助信息并退出进程
        commandParser.showVersion();
        Q_UNREACHABLE();
    }
    if (commandLineArgs.type != CommandType::DAEMON) {
        runClient(commandLineArgs.type, commandLineArgs.processName);
        return 0;
    }

    process_manager::ConfParser confParser("/etc/kylin-os-manager/process-manager.ini");
    if (confParser.parseError()) {
        std::cout << "Failed to parse configuration file." << std::endl;
        return -1;
    }

    process_manager::Daemon daemon(confParser.processConfs());
    daemon.run();

    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService("com.kylin.kom.process.manager")) {
        std::cout << "Register D-Bus service fail." << std::endl;
        return -1;
    }

    QObject object;
    process_manager::DBusService dbusService(&object, daemon);
    if (!conn.registerObject("/com/kylin/kom/process/manager", &dbusService, QDBusConnection::ExportAllSlots)) {
        std::cout << "Register D-Bus object fail." << std::endl;
        return -1;
    }

    app.exec();
}
