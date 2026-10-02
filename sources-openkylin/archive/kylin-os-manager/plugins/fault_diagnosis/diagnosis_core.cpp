#include "diagnosis_core.h"
#include <glib.h>
#include <QDebug>
#include <QHash>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace fault_diagnosis {

DiagnosisCore::DiagnosisCore()
    : diagnosis_manager_(nullptr)
{

}

void DiagnosisCore::InitI18n() {
    g_autoptr(GError) error = NULL;
    g_autoptr(GKeyFile) key_file = g_key_file_new();
    if (!g_key_file_load_from_file(key_file, "/usr/share/kylin-os-manager/fault-diagnosis/i18n.conf", G_KEY_FILE_NONE, &error)) {
        qCritical() << "Fault diagnosis load i18n file fail: " << error->message;
        return;
    }
    // 网络检测
    g_autofree gchar *network = g_key_file_get_locale_string(key_file, "Category", "Network", NULL, &error);
    if (network != NULL) {
        diagnosis_type_i18n_.insert(DiagnosisType::kNetwork, network);
    }
    // 系统组件与性能检测
    g_autofree gchar *performance_and_components = g_key_file_get_locale_string(key_file, "Category", "PerformanceAndComponents", NULL, &error);
    if (performance_and_components != NULL) {
        diagnosis_type_i18n_.insert(DiagnosisType::kPerformanceAndComponents, performance_and_components);
    }
    // 应用程序检测
    g_autofree gchar *application = g_key_file_get_locale_string(key_file, "Category", "Application", NULL, &error);
    if (application != NULL) {
        diagnosis_type_i18n_.insert(DiagnosisType::kApplication, application);
    }
    // 系统在线更新检测
    g_autofree gchar *system_update = g_key_file_get_locale_string(key_file, "Category", "SystemUpdate", NULL, &error);
    if (system_update != NULL) {
        diagnosis_type_i18n_.insert(DiagnosisType::kSystemUpdate, system_update);
    }
    // 软件商店检测
    g_autofree gchar *software_store = g_key_file_get_locale_string(key_file, "Category", "SoftwareStore", NULL, &error);
    if (software_store != NULL) {
        diagnosis_type_i18n_.insert(DiagnosisType::kSoftwareStore, software_store);
    }
    // 系统磁盘可用性检测
    g_autofree gchar *disk = g_key_file_get_locale_string(key_file, "Category", "Disk", NULL, &error);
    if (disk != NULL) {
        diagnosis_type_i18n_.insert(DiagnosisType::kDisk, disk);
    }
    // 有线网络检测
    g_autofree gchar *wired_network = g_key_file_get_locale_string(key_file, "Category", "WiredNetwork", NULL, &error);
    if (wired_network != NULL) {
        diagnosis_type_i18n_.insert(DiagnosisType::kWiredNetwork, wired_network);
    }
    // 无线网络检测
    g_autofree gchar *wireless_network = g_key_file_get_locale_string(key_file, "Category", "WirelessNetwork", NULL, &error);
    if (wireless_network != NULL) {
        diagnosis_type_i18n_.insert(DiagnosisType::kWirelessNetwork, wireless_network);
    }


    // 具体的诊断项
    // 能否访问内网
    g_autofree gchar *can_access_internal_network = g_key_file_get_locale_string(key_file, "DiagnosisItem", "CanAccessInternalNetwork", NULL, &error);
    if (can_access_internal_network != NULL) {
        diagnosis_item_i18n_.insert(DiagnosisItem::kCanAccessInternalNetwork, can_access_internal_network);
    }
}

void DiagnosisCore::Init() {
    if (diagnosis_manager_ != nullptr) {
        // 已经初始化过了
        return;
    }

    diagnosis_manager_ = new DiagnosisManage(this);
    connect(diagnosis_manager_, &DiagnosisManage::sig_CheckProgress, this, &DiagnosisCore::sig_CheckProgress);
    connect(diagnosis_manager_, &DiagnosisManage::sig_RepairProgress, this, &DiagnosisCore::sig_RepairProgress);
    connect(diagnosis_manager_, &DiagnosisManage::sig_Checked, this, &DiagnosisCore::sig_Checked);
    connect(diagnosis_manager_, &DiagnosisManage::sig_CheckFinish, this, &DiagnosisCore::sig_CheckFinished);
    connect(diagnosis_manager_, &DiagnosisManage::sig_Fixed, this, &DiagnosisCore::on_Fixed);
    connect(diagnosis_manager_, &DiagnosisManage::sig_RepairFinish, this, &DiagnosisCore::on_RepairFinish);
}

// 承担根据模式筛选诊断项的功能
void DiagnosisCore::on_DiagnosticsList(fault_diagnosis::Mode mode, QVariant data) {
    Init();
    Database::Instance();

    CategoryDiagnosticEntry category_diagnostic_entrys;

    auto diagnoses = diagnosis_manager_->Diagnoses();
    for (const auto &d: diagnoses) {
        QString diagnosis_mark = d->Mark();

        // 根据 mode 对诊断实例进行筛选
        switch (mode) {
            case Mode::kDiagnosisSpecificTypeMode:
                [[fallthrough]];
            case Mode::kNormalMode: {
                if (diagnosis_mark == "complete_machine") {
                    break;
                } else {
                    continue;
                }
            }
            case Mode::kRepairSpecificErrorMode: {
                if (diagnosis_mark == "specific_error") {
                    break;
                } else {
                    continue;
                }
            }
            default: {
                continue;
            }
        }

        QList<DiagnosticCategory> categories;
        d->DiagnosticsList(categories, data);

        // 对每个诊断实例所支持的诊断项，按照分类合并
        for (const auto &c: categories) {
            for (const auto &i: c.base_diagnostic_items) {
                DiagnosticEntry diagnostic_entry;
                diagnostic_entry.diagnosis_mark = diagnosis_mark;
                diagnostic_entry.item = i;
                category_diagnostic_entrys[c.category].push_back(diagnostic_entry);
            }
        }
    }

    // 筛选诊断项
    if (mode == Mode::kDiagnosisSpecificTypeMode) {
        auto type = data.value<enum DiagnosisType>();
        switch (type) {
            case DiagnosisType::kNetwork: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != diagnosis_type_i18n_[DiagnosisType::kNetwork]) {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            case DiagnosisType::kPerformanceAndComponents: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != diagnosis_type_i18n_[DiagnosisType::kPerformanceAndComponents]) {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            case DiagnosisType::kApplication: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != diagnosis_type_i18n_[DiagnosisType::kApplication]) {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            case DiagnosisType::kSystemUpdate: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != diagnosis_type_i18n_[DiagnosisType::kSystemUpdate]) {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            case DiagnosisType::kSoftwareStore: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != diagnosis_type_i18n_[DiagnosisType::kSoftwareStore]) {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            case DiagnosisType::kDisk: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != diagnosis_type_i18n_[DiagnosisType::kDisk]) {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            case DiagnosisType::kWiredNetwork: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != diagnosis_type_i18n_[DiagnosisType::kWiredNetwork]) {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            case DiagnosisType::kWirelessNetwork: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != diagnosis_type_i18n_[DiagnosisType::kWirelessNetwork]) {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    auto remove_internal_network_diagnosis = [](QList<DiagnosticEntry> &entrys) {
        for (auto iter = entrys.begin(); iter != entrys.end(); iter++) {
            if (iter->item.base.name == diagnosis_item_i18n_[DiagnosisItem::kCanAccessInternalNetwork]) {
                entrys.erase(iter);
                break;
            }
        }
    };
    if (!IsEnableInternalNetworkDiagnosis()) {
        auto wired_network_category_iter = category_diagnostic_entrys.find(diagnosis_type_i18n_[DiagnosisType::kWiredNetwork]);
        if (wired_network_category_iter != category_diagnostic_entrys.end()) {
            remove_internal_network_diagnosis(wired_network_category_iter.value());
        }
        auto wireless_network_category_iter = category_diagnostic_entrys.find(diagnosis_type_i18n_[DiagnosisType::kWirelessNetwork]);
        if (wireless_network_category_iter != category_diagnostic_entrys.end()) {
            remove_internal_network_diagnosis(wireless_network_category_iter.value());
        }
    }

    Q_EMIT sig_DiagnosticsListFinished(category_diagnostic_entrys);
}

void DiagnosisCore::on_Check(fault_diagnosis::CheckEntry check_entry) {
    Init();
    auto diagnosis = diagnosis_manager_->DiagnosisWithMark(check_entry.diagnosis_mark);
    if (!diagnosis) {
        throw std::runtime_error("diagnosis mark is not exist");
    }

    if (!diagnosis->Check(check_entry.entry_mark, check_entry.data)) {
        Q_EMIT sig_CheckFinished(check_entry.diagnosis_mark, check_entry.entry_mark, false);
    }
}

void DiagnosisCore::on_Repair(fault_diagnosis::RepairEntry repair_entry) {
    // 由于 SpecificError 和 CompleteMachine 实例是相同的修复逻辑
    // 所以此处做标记的替换
    auto diagnosis_mark = repair_entry.diagnosis_mark;
    if (diagnosis_mark == "specific_error") {
        diagnosis_mark = "complete_machine";
    }
    auto diagnosis = diagnosis_manager_->DiagnosisWithMark(diagnosis_mark);
    if (!diagnosis) {
        throw std::runtime_error("diagnosis mark is not exist");
    }

    QList<RepairItem> repair_items;
    for (const auto &error: repair_entry.errors) {
        RepairItem repair_item;
        repair_item.error_code = error;
        repair_items.push_back(repair_item);
    }

    if (!diagnosis->Repair(repair_entry.entry_mark, repair_items)) {
        on_RepairFinish(repair_entry.diagnosis_mark, repair_entry.entry_mark, false);
    }
}

void DiagnosisCore::on_RepairFinish(QString mark, QString base_diagnostic_mark, bool ok) {
    RepairEntryResult repair_entry_result;
    repair_entry_result.diagnosis_mark = mark;
    repair_entry_result.entry_mark = base_diagnostic_mark;
    repair_entry_result.ok = ok;
    if (!ok) {
        repair_entry_result.reason = tr("Repair fail");
    }

    Q_EMIT sig_RepairFinished(repair_entry_result);
}

void DiagnosisCore::on_Fixed(QString mark, QString error_code, bool ok, QString reason, qint64 solution_type, QString solution_context) {
    Q_EMIT sig_Fixed(mark, error_code, ok, reason, solution_type, solution_context);
}

/*
 * 用户级配置文件：~/.config/kylin-os-manager/net-check/kylin-netcheck-tools.conf
 * 系统级配置文件：/etc/kylin-os-manager/net-check/kylin-netcheck-tools.conf
 * 用户级配置文件优先级高
 */
bool DiagnosisCore::IsEnableInternalNetworkDiagnosis() const {
    auto parser_internal_enable_state = [](QString path) -> bool {
        QFile file(path);
        if (!file.exists()) {
            return false;
        }
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Get internal network enable state open file fail [ " << path << " ]";
            return false;
        }
        const auto content = file.readAll();
        file.close();

        QJsonParseError error;
        const auto doc = QJsonDocument::fromJson(content, &error);
        if (doc.isNull()) {
            qCritical() << "Get internal network enable state parser json fail: " << error.errorString() << "[ " << path << " ]";
            return false;
        }
        if (!doc.isObject()) {
            qCritical() << "Get internal network enable state json format error";
            return false;
        }

        const auto root = doc.object();
        const auto usual_iter = root.find("config-usual");
        if (usual_iter == root.end()) {
            qWarning() << "Get internal network enabel state usual field is not exist";
            return false;
        }
        if (!usual_iter.value().isObject()) {
            qWarning() << "Get internal network enable state usual field value format error";
            return false;
        }

        const auto object = usual_iter.value().toObject();
        const auto switch_iter = object.find("switch");
        if (switch_iter == object.end()) {
            qWarning() << "Get internal network enable state switch field is not exist";
            return false;
        }

        return switch_iter.value().toBool();
    };

    auto home_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::HomeLocation);
    auto user_conf_file = QString("%1/.config/kylin-os-manager/net-check/kylin-netcheck-tools.conf").arg(home_path);
    QString system_conf_file{"/etc/kylin-os-manager/net-check/kylin-netcheck-tools.conf"};

    QFileInfo user_conf_file_info(user_conf_file);
    if (user_conf_file_info.exists()) {
        return parser_internal_enable_state(user_conf_file);
    } else {
        return parser_internal_enable_state(system_conf_file);
    }
}

const QMap<enum DiagnosisType, QString> &DiagnosisCore::DiagnosisTypeI18n() {
    return diagnosis_type_i18n_;
}

}
