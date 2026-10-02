#include "tool_box_interface.h"
#include <QProcess>
#include <QProcessEnvironment>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>
#include "kom-service.h"

namespace tool_box
{

ToolBoxAdaptor::ToolBoxAdaptor(QObject *obj) : QObject(obj) {}

void ToolBoxAdaptor::UpdateSource()
{
    if (!isAllowedCaller()) {
        return;
    }
    ok_ = true;
    standard_output_buf_.clear();
    standard_error_buf_.clear();

    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert("LANGUAGE", "en_US");

    auto *process = new QProcess;
    process->setProgram("apt");
    process->setArguments(QStringList{"update"});
    process->setProcessEnvironment(environment);
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &ToolBoxAdaptor::on_UpdateSourceFinish);
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        standard_output_buf_.append(process->readAllStandardOutput());
        HandleOutput(standard_output_buf_);
    });
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        standard_error_buf_.append(process->readAllStandardError());
        HandleOutput(standard_error_buf_);
    });
    process->start();
}

void ToolBoxAdaptor::on_UpdateSourceFinish(int exit_code, [[maybe_unused]]QProcess::ExitStatus exit_status) {
    sender()->deleteLater();

    auto message = QDBusMessage::createSignal(TOOL_BOX_DBUS_PATH, TOOL_BOX_DBUS_INTERFACE, "UpdateSourceFinish");
    QList<QVariant> args;
    if (!exit_code && !ok_) {
        args.push_back(-1);
    } else {
        args.push_back(exit_code);
    }
    message.setArguments(args);
    QDBusConnection::systemBus().send(message);
}

void ToolBoxAdaptor::HandleOutput(QByteArray &output) {
    while (true) {
        if (output.isEmpty()) {
            break;
        }

        auto index = output.indexOf('\n');
        if (index == -1) {
            // 不足一行直接退出
            break;
        }

        // 取出 '\n' 前的数据
        auto line = output.mid(0, index);
        qInfo() << line;
        if (line.contains("Err:") ||
            line.contains("Temporary failure resolving"))
        {
            ok_ = false;
        }
        // 移除掉到 '\n' 的数据
        output.remove(0, index + 1);
    }
}

bool ToolBoxAdaptor::isAllowedCaller()
{
    return true;
    QString clientService = message().service();
    QDBusConnectionInterface *intf = connection().interface();
    unsigned int pid = intf->servicePid(clientService).value();
    /*check caller environ*/
    if (!kom::KomService::checkEnviron(pid)) {
        sendErrorReply(QDBusError::ErrorType::Failed, QString("[%2] dbus method control,env forbidden").arg((pid)));
        return false;
    }
    /*check caller white list*/
    QFileInfo file(QString("/proc/%1/exe").arg(pid));
    QString execPath = "";
    if (file.exists()) {
        execPath = file.canonicalFilePath();
        foreach (const QString &whitePath, kom::KomService::programWhiteList) {
            if (execPath == whitePath) {
                return true;
            }
        }
    }
    sendErrorReply(QDBusError::ErrorType::Failed,
                   QString("[%2] dbus method control,operation not permitted").arg((execPath)));
    return false;
}

} // namespace tool_box
