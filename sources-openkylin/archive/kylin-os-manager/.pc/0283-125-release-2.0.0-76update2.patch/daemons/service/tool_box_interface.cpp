#include "tool_box_interface.h"
#include <QProcess>
#include <QProcessEnvironment>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>
#include <QSettings>
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

void ToolBoxAdaptor::ChangePlace(int place) {
    qInfo() << "park switch: " << place;

    QSettings s("/usr/share/kylin-os-manager/treasure-chest/kylin-icbc-tool/icbc-tool.conf", QSettings::IniFormat);
    s.beginGroup("Change Place");
    QStringList tmpKeys = s.allKeys();
    tmpKeys.removeAll("state");

    // 读取 hosts 文件内容
    QFile hosts("/etc/hosts");
    if (!hosts.open(QIODevice::Text | QIODevice::ReadOnly)) {
        qCritical() << "park switch fail: open hosts file fail";
        return;
    }
    QString hostsTmp = hosts.readAll();
    hosts.close();

    // 提取 hosts 文件里不在配置文件中的域名
    QStringList hostsListNew;
    QStringList hostsListOld = hostsTmp.split("\n");
    for (QString hname : hostsListOld) {
        bool ok = true;
        for (QString name: tmpKeys) {
            if (hname.contains(name)){
                ok = false;
                break;
            }
        }
        if (ok) {
            hostsListNew.append(hname);
        }
    }
    if (hostsListNew.endsWith("")) {
        hostsListNew.removeLast();
    }

    // 提取配置文件中的域名
    bool success = true;
    if (place > 0) {
        for (QString name: tmpKeys) {
            QStringList tmplist = s.value(name).toStringList();
            if (tmplist.size() < place) {
                qCritical() << "park switch fail: conf domain [" << name <<"] ip lack";
                success = false;
                break;
            }
            QString tmpStr = tmplist.at(place - 1) + " " +  name;
            hostsListNew.append(tmpStr);
        }
    }

    // 将整合后的域名重新写入 hosts 文件
    if (success) {
        if (!hosts.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
            qCritical() << "park switch fail: modify hosts file open fail";
            return;
        }

        for (QString hname: hostsListNew) {
            hosts.write(hname.toLocal8Bit() + '\n');
        }
        hosts.close();
        s.setValue("state",place);
    }

    s.endGroup();
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
