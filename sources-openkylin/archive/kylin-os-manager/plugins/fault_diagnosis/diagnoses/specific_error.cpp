#include "specific_error.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace fault_diagnosis {

static const QString DBUS_NAME = "com.kylin.kydoctor";
static const QString DBUS_PATH = "/com/kylin/kydoctor";
static const QString DBUS_INTERFACE = "com.kylin.kydoctor";

SpecificError::SpecificError(QObject *parent)
    : Diagnosis(parent)
{
    QDBusConnection::systemBus().connect(DBUS_NAME,
                                         DBUS_PATH,
                                         DBUS_INTERFACE,
                                         "CheckItemNonU",
                                         this,
                                         SLOT(on_Checked(QString)));
    QDBusConnection::systemBus().connect(DBUS_NAME,
                                         DBUS_PATH,
                                         DBUS_INTERFACE,
                                         "CheckItemNonUOver",
                                         this,
                                         SLOT(on_CheckFinish(bool)));
}

SpecificError::~SpecificError() {

}

QString SpecificError::Mark() {
    return "specific_error";
}

bool SpecificError::DiagnosticsList(QList<fault_diagnosis::DiagnosticCategory> &diagnostic_categories, QVariant data) {
    return true;
}

bool SpecificError::Check(const QString &base_diagnostic_mark, QVariant data) {
    bool ok;
    qint64 error_code = base_diagnostic_mark.toLongLong(&ok, 0);
    if (!ok) {
        qCritical() << "Specific error check switch error code fail: [ " << base_diagnostic_mark << " ]";
        return false;
    }

    current_diagnostic_mark_ = base_diagnostic_mark;

    auto message = QDBusMessage::createMethodCall(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, "CheckItemNonUniversal");
    message.setArguments({error_code, data});
    auto reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Specific error check [" << base_diagnostic_mark << "] data [" << data << "] d-bus interface return error: " << reply.errorMessage();
        return false;
    }
    return true;
}

// 走 CompleteMachine 的 Repair 函数
bool SpecificError::Repair(const QString &base_diagnostic_mark, const QList<fault_diagnosis::RepairItem> &repair_item) {
    return true;
}

void SpecificError::on_Checked(QString result) {
    DiagnosticResult diagnostic_result;

    do {
        QJsonParseError parse_error;
        const auto doc = QJsonDocument::fromJson(result.toLocal8Bit(), &parse_error);
        if (doc.isNull()) {
            qCritical() << "Specific error diagnosis parse check result json fail: " << parse_error.errorString();
            break;
        }
        if (!doc.isObject()) {
            qCritical() << "Specific error diagnosis check result json format error.";
            break;
        }
        const auto root = doc.object();
        if (root.find("Details") == root.end()) {
            qCritical() << "Specific error diagnosis check result json missing 'Details' field.";
            break;
        }
        const auto details = root.value("Details");
        if (!details.isArray()) {
            qCritical() << "Specific error diagnosis check result 'Details' field format error.";
            break;
        }
        const auto errors = details.toArray();
        for (auto error_iter = errors.constBegin(); error_iter != errors.constEnd(); error_iter++) {
            if (!error_iter->isObject()) {
                qWarning() << "Specific error diagnosis 'Details' field element format error.";
                continue;
            }
            const auto error = error_iter->toObject();
            if (error.find("Name") == error.end() ||
                error.find("ErrorCode") == error.end() ||
                error.find("Describe") == error.end() ||
                error.find("Context") == error.end() ||
                error.find("CanRepair") == error.end() ||
                error.find("NetworkState") == error.end())
            {
                qWarning() << "Specific error diagnosis 'Details' field element missing field.";
                continue;
            }

            SpecificErrorR specific_error_r;
            specific_error_r.name = error.value("Name").toString();
            specific_error_r.error_code = error.value("ErrorCode").toString();
            specific_error_r.desc = error.value("Describe").toString();
            specific_error_r.context = error.value("Context").toString();
            specific_error_r.can_repair = error.value("CanRepair").toString();
            specific_error_r.network_state = error.value("NetworkState").toString();
            diagnostic_result.specific_errors.push_back(std::move(specific_error_r));
        }
    } while (0);

    Q_EMIT sig_Checked(Mark(), diagnostic_result);
}

void SpecificError::on_CheckFinish(bool ok) {
    Q_EMIT sig_CheckFinish(Mark(), current_diagnostic_mark_, ok);
}

}
