#include <sstream>
#include <iomanip>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QByteArray>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnection>
#include <QDebug>
#include "complete_machine.h"

static const QString DBUS_NAME = "com.kylin.kydoctor";
static const QString DBUS_PATH = "/com/kylin/kydoctor";
static const QString DBUS_INTERFACE = "com.kylin.kydoctor";

namespace fault_diagnosis {

CompleteMachine::CompleteMachine(QObject *parent)
    : Diagnosis(parent)
{
    interface_.reset(new QDBusInterface(DBUS_NAME,
                                        DBUS_PATH,
                                        DBUS_INTERFACE,
                                        QDBusConnection::systemBus(),
                                        this));
    if (interface_->isValid()) {
        QDBusConnection::systemBus().connect(DBUS_NAME,
                                             DBUS_PATH,
                                             DBUS_INTERFACE,
                                             "SignalProgress",
                                             this,
                                             SLOT(on_RepairProgress(qint64,qint32)));
        QDBusConnection::systemBus().connect(DBUS_NAME,
                                             DBUS_PATH,
                                             DBUS_INTERFACE,
                                             "SigFixed",
                                             this,
                                             SLOT(on_Fixed(qint64,bool,QString,qint64,QString)));
        QDBusConnection::systemBus().connect(DBUS_NAME,
                                             DBUS_PATH,
                                             DBUS_INTERFACE,
                                             "CheckItem",
                                             this,
                                             SLOT(on_Checked(QString)));
        QDBusConnection::systemBus().connect(DBUS_NAME,
                                             DBUS_PATH,
                                             DBUS_INTERFACE,
                                             "CheckItemOver",
                                             this,
                                             SLOT(on_CheckFinish(bool)));
        QDBusConnection::systemBus().connect(DBUS_NAME,
                                             DBUS_PATH,
                                             DBUS_INTERFACE,
                                             "RepairOver",
                                             this,
                                             SLOT(on_RepairFinish(bool)));
    }
}

CompleteMachine::~CompleteMachine() {

}

QString CompleteMachine::Mark() {
    return "complete_machine";
}

bool CompleteMachine::DiagnosticsList(QList<DiagnosticCategory> &diagnostic_categories, [[maybe_unused]]QVariant data) {
    if (!interface_->isValid()) {
        qCritical() << "Complete machine diagnosis dbus interface is not valid: " << interface_->lastError();
        return false;
    }

    // 0: 主线
    // 1: 工行
#if defined(ICBC_ENVIRONMENT)
    QDBusReply<QString> reply = interface_->call("GetCheckList", qint64(1));
#else
    QDBusReply<QString> reply = interface_->call("GetCheckList", qint64(0));
#endif
    if (!reply.isValid()) {
        qCritical() << "Complete machine diagnosis DiagnosticsList dbus interface return is not vaild";
        return false;
    }

    QJsonParseError json_parse_error;
    const auto doc= QJsonDocument::fromJson(reply.value().toUtf8(), &json_parse_error);
    if (doc.isNull()) {
        qCritical() << "Complete machine diagnosis parse json fail: " << json_parse_error.errorString();
        return false;
    }
    if (!doc.isArray()) {
        qCritical() << "Complete machine diagnosis diagnostics list json format error.";
        return false;
    }

    const auto root_array = doc.array();
    for (auto i = 0; i < root_array.size(); i++) {
        if (!root_array.at(i).isObject()) {
            continue;
        }

        const auto object = root_array.at(i).toObject();
        const auto class_a_iter = object.find("class_a");
        const auto class_b_iter = object.find("class_b");
        if (class_a_iter == object.end() ||
            class_b_iter == object.end())
        {
            continue;
        }
        if (!class_b_iter.value().isArray()) {
            qWarning() << "Complete machine diagnosis diagnostics list json class_b field format error";
            continue;
        }

        const auto class_b_array = class_b_iter.value().toArray();
        if (class_b_array.empty()) {
            qWarning() << "Complete machine diagnosis diagnostics list json class_b field element is empty";
            continue;
        }

        DiagnosticCategory diagnostic_category;
        for (const auto &b: class_b_array) {
            if (!b.isObject()) {
                continue;
            }

            const auto class_b_obj = b.toObject();
            const auto class_b_name_iter = class_b_obj.find("Name");
            const auto class_b_desc_iter = class_b_obj.find("Describe");
            const auto class_b_id_iter = class_b_obj.find("class_b_id");
            if (class_b_name_iter == class_b_obj.end() ||
                class_b_desc_iter == class_b_obj.end() ||
                class_b_id_iter == class_b_obj.end())
            {
                continue;
            }

            // 有效的基础检测项
            BaseDiagnosticItem base_diagnostic_item;
            base_diagnostic_item.base.name = class_b_name_iter.value().toString();
            base_diagnostic_item.base.describe = class_b_desc_iter.value().toString();
            base_diagnostic_item.base.mark = class_b_id_iter.value().toString();

            const auto class_c_iter = class_b_obj.find("class_c");
            if (class_c_iter != class_b_obj.end() &&
                class_c_iter.value().isArray())
            {
                const auto class_c_array = class_c_iter.value().toArray();
                for (const auto &c: class_c_array) {
                    if (!c.isObject()) {
                        continue;
                    }

                    const auto class_c_obj = c.toObject();
                    const auto class_c_name_iter = class_c_obj.find("Name");
                    const auto class_c_desc_iter = class_c_obj.find("Describe");
                    if (class_c_name_iter == class_c_obj.end() ||
                        class_c_desc_iter == class_c_obj.end())
                    {
                        continue;
                    }

                    // 有效的三级检测项
                    DiagnosticItem diagnostic_item;
                    diagnostic_item.name = class_c_name_iter.value().toString();
                    diagnostic_item.describe = class_c_desc_iter.value().toString();
                    diagnostic_item.mark = diagnostic_item.name;
                    base_diagnostic_item.subitem.push_back(std::move(diagnostic_item));
                }
            }
            diagnostic_category.base_diagnostic_items.push_back(std::move(base_diagnostic_item));
        }

        if (!diagnostic_category.base_diagnostic_items.empty()) {
            // 有效的一级分类
            diagnostic_category.category = class_a_iter.value().toString();
            diagnostic_categories.push_back(std::move(diagnostic_category));
        }
    }

    return true;
}

bool CompleteMachine::Check(const QString &base_diagnostic_mark, [[maybe_unused]]QVariant data) {
    if (!interface_->isValid()) {
        qCritical() << "Complete machine diagnosis dbus interface is not vaild";
        return false;
    }

    current_diagnosis_base_diagnostic_mark_ = base_diagnostic_mark;

    bool ok;
    qint64 mark1 = base_diagnostic_mark.toLongLong(&ok, 0);
    if (!ok) {
        qCritical() << "Complete machine diagnosis Check conversion base entry mark fail";
        return false;
    }

    auto message = interface_->call("CheckItem", mark1);
    if (message.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Complete machine diagnosis CheckItem dbus interface return error: " << message.errorMessage();
        return false;
    }

    return true;
}

bool CompleteMachine::Repair(const QString &base_diagnostic_mark, const QList<RepairItem> &repair_items) {
    if (!interface_->isValid()) {
        qCritical() << "Complete machine diagnosis dbus interface is not valid";
        return false;
    }

    current_repair_base_diagnostic_mark_ = base_diagnostic_mark;

    QJsonArray root_array;
    for (const auto &i: repair_items) {
        QJsonObject obj;
        obj["ErrorCode"] = i.error_code;
        root_array.push_back(obj);
    }

    QJsonDocument doc;
    doc.setArray(root_array);
    auto json_data = doc.toJson();

    auto message = interface_->call("RepairItem", QString::fromLocal8Bit(json_data));
    if (message.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Complete machine diagnosis repair dbus return error message: " << message.errorMessage();
        return false;
    }

    return true;
}

void CompleteMachine::on_CheckProgress(qint64 base_diagnostic_mark, qint32 progress) {
    Q_EMIT sig_CheckProgress(Mark(), base_diagnostic_mark, progress);
}

void CompleteMachine::on_RepairProgress(qint64 base_diagnostic_mark, qint32 progress) {
    Q_EMIT sig_RepairProgress(Mark(), base_diagnostic_mark, progress);
}

void CompleteMachine::on_Checked(QString check_result) {
    DiagnosticResult diagnostic_result;

    QJsonParseError json_parse_error;
    const auto doc = QJsonDocument::fromJson(check_result.toLocal8Bit(), &json_parse_error);
    if (doc.isNull()) {
        qCritical() << "Complete machine diagnosis parse check result json fail: " << json_parse_error.errorString();
        return;
    }
    if (!doc.isObject()) {
        qCritical() << "Complete machine diagnosis check result json format error";
        return;
    }

    const auto obj = doc.object();
    const auto class_b_id_iter = obj.find("class_b_id");
    const auto error_code_iter = obj.find("ErrorCode");
    const auto describe_iter = obj.find("Describe");
    if (class_b_id_iter == obj.end() ||
        error_code_iter == obj.end() ||
        describe_iter == obj.end())
    {
        qCritical() << "Complete machine diagnosis check result json field lack";
        return;
    }

    diagnostic_result.base_entry_mark = class_b_id_iter.value().toString();
    diagnostic_result.error_code = error_code_iter.value().toString();
    diagnostic_result.describe = describe_iter.value().toString();

    const auto details_iter = obj.find("Details");
    if (details_iter != obj.end() &&
        details_iter.value().isArray())
    {
        const auto details = details_iter.value().toArray();
        for (const auto &i: details) {
            if (!i.isObject()) {
                continue;
            }

            const auto subitem = i.toObject();
            const auto class_c_iter = subitem.find("class_c");
            const auto subitem_error_code_iter = subitem.find("ErrorCode");
            const auto subitem_describe_iter = subitem.find("Describe");
            if (class_c_iter == subitem.end() ||
                subitem_error_code_iter == subitem.end() ||
                subitem_describe_iter == subitem.end())
            {
                continue;
            }

            SubitemError subitem_error;
            subitem_error.mark = class_c_iter.value().toString();
            subitem_error.error_code = subitem_error_code_iter.value().toString();
            subitem_error.describe = subitem_describe_iter.value().toString();
            diagnostic_result.details.push_back(std::move(subitem_error));
        }
    }

    const auto overview_iter = obj.find("Overview");
    if (overview_iter != obj.end()) {
        diagnostic_result.overview = overview_iter.value().toString();
    }
    const auto class_a_iter = obj.find("class_a");
    if (class_a_iter != obj.end()) {
        diagnostic_result.category = class_a_iter.value().toString();
    }
    const auto class_b_iter = obj.find("class_b");
    if (class_b_iter != obj.end()) {
        diagnostic_result.base_entry_name = class_b_iter.value().toString();
    }

    Q_EMIT sig_Checked(Mark(), diagnostic_result);
}

void CompleteMachine::on_CheckFinish(bool ok) {
    Q_EMIT sig_CheckFinish(Mark(), current_diagnosis_base_diagnostic_mark_, ok);
}

void CompleteMachine::on_Fixed(qint64 error_code, bool ok, QString reason, qint64 solution_type, QString solution_context) {
    // 将错误码从 qint64 类型转换成 QString 类型
    // 错误码规范：使用 "0x" 前缀，数值部分共 16 个字符，不足左侧补 '0'
    std::stringstream ss;
    ss << "0x" << std::setfill('0') << std::setw(16) << std::hex << error_code;
    if (current_repair_base_diagnostic_mark_ == "specific_error") {
        Q_EMIT sig_Fixed("specific_error", QString::fromStdString(ss.str()), ok, reason, solution_type, solution_context);
    } else {
        Q_EMIT sig_Fixed(Mark(), QString::fromStdString(ss.str()), ok, reason, solution_type, solution_context);
    }
}

void CompleteMachine::on_RepairFinish(bool ok) {
    if (current_repair_base_diagnostic_mark_ == "specific_error") {
        Q_EMIT sig_RepairFinish("specific_error", current_repair_base_diagnostic_mark_, ok);
    } else {
        Q_EMIT sig_RepairFinish(Mark(), current_repair_base_diagnostic_mark_, ok);
    }
}

}
