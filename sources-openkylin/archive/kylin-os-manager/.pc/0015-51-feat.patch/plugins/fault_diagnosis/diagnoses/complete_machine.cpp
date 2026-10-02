#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QByteArray>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnection>
#include <QDebug>
#include "complete_machine.h"

namespace fault_diagnosis {

CompleteMachine::CompleteMachine(QObject *parent)
    : Diagnosis(parent)
{
    interface_.reset(new QDBusInterface("com.kylin.kydoctor",
                                        "/com/kylin/kydoctor",
                                        "com.kylin.kydoctor",
                                        QDBusConnection::systemBus(),
                                        this));
    if (interface_->isValid()) {
        QDBusConnection::systemBus().connect("com.kylin.kydoctor",
                                             "/com/kylin/kydoctor",
                                             "com.kylin.kydoctor",
                                             "SignalProgress",
                                             this,
                                             SLOT(on_RepairProgress(qint64,qint32)));
        QDBusConnection::systemBus().connect("com.kylin.kydoctor",
                                             "/com/kylin/kydoctor",
                                             "com.kylin.kydoctor",
                                             "SigFixed",
                                             this,
                                             SLOT(on_Fixed(qint64,bool,QString)));
    }
}

CompleteMachine::~CompleteMachine() {

}

QString CompleteMachine::Mark() {
    return "complete_machine";
}

bool CompleteMachine::DiagnosticsList(QList<DiagnosticCategory> &diagnostic_categories) {
    if (!interface_->isValid()) {
        qCritical() << "Complete machine diagnosis dbus interface is not valid: " << interface_->lastError();
        return false;
    }

    QDBusReply<QString> reply = interface_->call("GetCheckList");
    if (!reply.isValid()) {
        qCritical() << "Complete machine diagnosis DiagnosticsList dbus interface return is not vaild";
        return false;
    }

    // 测试数据
    // QByteArray tmp_json_data{"[{\"Category\":\"Security\",\"Types\":[{\"Type\":1234,\"Name\":\"cve 检测\",\"Describe\":\"检测 cve 漏洞\"},{\"Type\":9876,\"Name\":\"kve 检测\",\"Describe\":\"检测 kve 漏洞\"}]},{\"Category\":\"Hardware\",\"Types\":[{\"Type\":1111,\"Name\":\"硬盘检测\",\"Describe\":\"检测硬盘是否正常\"},{\"Type\":2222,\"Name\":\"蓝牙检测\",\"Describe\":\"检测蓝牙是否正常\"}]}]"};

    QJsonParseError json_parse_error;
    auto document= QJsonDocument::fromJson(reply.value().toUtf8(), &json_parse_error);
    if (document.isNull()) {
        qCritical() << "Complete machine diagnosis parse json fail: " << json_parse_error.errorString();
        return false;
    }
    if (!document.isArray()) {
        qCritical() << "Complete machine diagnosis diagnostics list json format error.";
        return false;
    }

    const auto root_array = document.array();
    for (auto i = 0; i < root_array.size(); i++) {
        if (!root_array.at(i).isObject())
            continue;

        const auto object = root_array.at(i).toObject();
        if (!object.contains("Types") || !object.contains("Category"))
            continue;

        const auto types = object["Types"];
        if (!types.isArray())
            continue;

        QList<DiagnosticItem> diagnostic_items;
        const auto type_array = types.toArray();
        for (auto j = 0; j < type_array.size(); j++) {
            if (!type_array.at(j).isObject())
                continue;

            const auto obj = type_array.at(j).toObject();
            if (!obj.contains("Type") || !obj.contains("Name") || !obj.contains("Describe"))
                continue;

            bool ok;
            qint64 type = obj["Type"].toString().toLongLong(&ok, 0);
            if (ok) {
                DiagnosticItem diagnostic_item;
                diagnostic_item.mark = type;
                diagnostic_item.name = obj["Name"].toString();
                diagnostic_item.describe = obj["Describe"].toString();
                diagnostic_items.push_back(diagnostic_item);

            }
        }

        DiagnosticCategory diagnostic_category = {object["Category"].toString(), diagnostic_items};
        diagnostic_categories.push_back(diagnostic_category);
    }

    return true;
}

bool CompleteMachine::Check(qint64 mark, DiagnosticResult &diagnostic_result) {
    if (!interface_->isValid()) {
        qCritical() << "Complete machine diagnosis dbus interface is not vaild";
        return false;
    }

    QDBusReply<QString> reply = interface_->call("CheckItem", mark);
    if (!reply.isValid()) {
        qCritical() << "Complete machine diagnosis CheckItem dbus interface return is not vaild";
        return false;
    }

    // 测试数据
    // QByteArray tmp_json_data{"{\"Overview\":\"检测了 10 项，7 项正确，3 项错误\",\"Details\":[{\"ErrorCode\":123456,\"Describe\":\"错误原因描述\"},{\"ErrorCode\":9876,\"Describe\":\"错误原因描述\"}]}"};

    QJsonParseError json_parse_error;
    const auto document = QJsonDocument::fromJson(reply.value().toUtf8(), &json_parse_error);
    if (document.isNull()) {
        qCritical() << "Complete machine diagnosis parse check result json fail: " << json_parse_error.errorString();
        return false;
    }
    if (!document.isObject()) {
        qCritical() << "Complete machine diagnosis check result json format error.";
        return false;
    }

    const auto root_object = document.object();
    if (!root_object.contains("Overview") || !root_object.contains("Details")) {
        qCritical() << "Complete machine diagnosis check result json incomplete fields.";
        return false;
    }

    const auto details = root_object["Details"];
    if (!details.isArray()) {
        qCritical() << "Complete machine diagnosis check result json \"Details\" fields format error.";
        return false;
    }

    QList<DiagnosticError> diagnostic_errors;
    const auto detail_array = details.toArray();
    for (int i = 0; i < detail_array.size(); i++) {
        if (!detail_array.at(i).isObject())
            continue;

        const auto obj = detail_array.at(i).toObject();
        if (!obj.contains("ErrorCode") || !obj.contains("Describe") || !obj.contains("Name")) {
            qWarning() << "Check D-Bus return value \"Details\" field is invalid element.";
            continue;
        }

        bool ok;
        qint64 error_code = obj["ErrorCode"].toString().toLongLong(&ok, 0);
        if (ok) {
            DiagnosticError diagnostic_error;
            diagnostic_error.error_code = error_code;
            diagnostic_error.name = obj["Name"].toString();
            diagnostic_error.describe = obj["Describe"].toString();
            diagnostic_errors.push_back(diagnostic_error);
        }
    }

    diagnostic_result.overview = root_object["Overview"].toString();
    diagnostic_result.details = diagnostic_errors;

    return true;
}

bool CompleteMachine::Repair(const QList<RepairItem> &repair_items) {
    if (!interface_->isValid()) {
        qCritical() << "Complete machine diagnosis dbus interface is not valid";
        return false;
    }

    QJsonArray root_array;
    for (const auto &i: repair_items) {
        QJsonObject obj;
        obj["ErrorCode"] = i.error_code;
        obj["SolutionType"] = i.solution_type;
        obj["Crc"] = i.crc;
        obj["CmdLine"] = i.cmdline;
        obj["PackageList"] = i.package_list;
        obj["OtherInfo"] = i.other;
        root_array.push_back(obj);
    }

    QJsonDocument doc;
    doc.setArray(root_array);
    auto json_data = doc.toJson();

    QEventLoop event_loop;
    bool ok;
    auto pend = interface_->asyncCall("RepairItem", QString::fromLocal8Bit(json_data));
    auto *watcher = new QDBusPendingCallWatcher(pend, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [&](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<bool> reply = *watcher;
        if (reply.isError()) {
            qCritical() << "Complete machine diagnosis RepairItem interface return is not valid: " << reply.error();
            ok = false;
        } else {
            ok = reply.argumentAt<0>();
        }
        watcher->deleteLater();
        event_loop.quit();
    });
    event_loop.exec();

    return ok;
}

void CompleteMachine::on_CheckProgress(qint64 item_mark, qint32 progress) {
    Q_EMIT sig_CheckProgress(Mark(), item_mark, progress);
}

void CompleteMachine::on_RepairProgress(qint64 item_mark, qint32 progress) {
    Q_EMIT sig_RepairProgress(Mark(), item_mark, progress);
}

void CompleteMachine::on_Fixed(qint64 error_code, bool ok, QString reason) {
    Q_EMIT sig_Fixed(Mark(), error_code, ok, reason);
}

}
